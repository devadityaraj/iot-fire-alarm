#include "AlarmManager.h"
#include "SensorManager.h"
#include "LEDManager.h"
#include "Utils.h"
#include "Pins.h"

static const char *TAG = "Alarm";

enum class AlarmPhase : uint8_t { STABLE, ACTIVE, RESET_COOLDOWN };

static AlarmPhase s_phase = AlarmPhase::STABLE;
static AlertType s_alertType = AlertType::NONE;
static uint32_t s_resetStartedMs = 0;

static bool s_buzzerOn = false;

static bool s_lastButtonReading = LOW;   // LOW = idle for active-HIGH button
static bool s_buttonStableState = LOW;
static uint32_t s_lastDebounceMs = 0;

static void setBuzzer(bool on) {
  if (on != s_buzzerOn) {
    digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
    s_buzzerOn = on;
  }
}

// Returns true on the debounced rising edge (LOW→HIGH) of the button.
// Button is active-HIGH: HIGH = pressed, LOW = idle.
static bool detectButtonPressEdge() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != s_lastButtonReading) {
    s_lastDebounceMs = millis();
  }
  s_lastButtonReading = reading;

  if ((millis() - s_lastDebounceMs) > Cfg::BUTTON_DEBOUNCE_MS) {
    if (reading != s_buttonStableState) {
      s_buttonStableState = reading;
      if (s_buttonStableState == HIGH) return true;  // rising edge = press
    }
  }
  return false;
}

static AlertType evaluateCandidateAlertType() {
  if (SensorManager::isFlameDetected()) return AlertType::FIRE;
  if (SensorManager::isSmokeAboveThreshold()) return AlertType::SMOKE;
  if (SensorManager::isTempHigh()) return AlertType::HIGH_TEMP;
  return AlertType::NONE;
}

static String buildAlertMessage(AlertType type) {
  switch (type) {
    case AlertType::FIRE:
      return "FIRE ALERT: Flame detected. Immediate action required.";
    case AlertType::SMOKE:
      return "SMOKE ALERT: Smoke level above safe threshold.";
    case AlertType::HIGH_TEMP:
      return "HIGH TEMPERATURE ALERT: Temperature exceeded safe limit.";
    default:
      return "Alert condition detected.";
  }
}

static void queueFirebaseAlert(bool active, AlertType type, uint32_t epoch) {
  SensorReadings r = SensorManager::getReadings();
  FirebaseUpdateItem item{};
  item.isAlertUpdate = true;
  item.temperature = r.temperature;
  item.humidity = r.humidity;
  item.smokeRaw = r.smokeRaw;
  item.flameActive = r.flameDetected;
  item.alertActive = active;
  item.alertType = type;
  item.epochTime = epoch;
  pushFirebaseUpdate(item);
}

// Sends alarm notification immediately. Called only on first trigger or priority escalation
// so Firebase and Telegram are notified exactly once per alarm event, not on every tick.
static void notify() {
  uint32_t epoch = nowEpoch();
  queueFirebaseAlert(true, s_alertType, epoch);
  pushTelegramMessage(buildAlertMessage(s_alertType));
  SharedState::setAlert(true, s_alertType, epoch);
}

// BUG-B + BUG-F FIX: accepts a log reason and Telegram message so each call site
// provides its own text. Prevents double-messages and makes Serial logs accurate.
static void triggerReset(const char* logReason, const char* telegramMsg) {
  LOG_I(TAG, "Alarm reset [%s] - entering 20s cooldown window", logReason);
  s_phase = AlarmPhase::RESET_COOLDOWN;
  s_alertType = AlertType::NONE;
  s_resetStartedMs = millis();
  setBuzzer(false);
  LEDManager::setMode(LEDMode::RESET_BREATHING);

  uint32_t epoch = nowEpoch();
  queueFirebaseAlert(false, AlertType::NONE, epoch);
  pushTelegramMessage(telegramMsg);
  SharedState::setAlert(false, AlertType::NONE, epoch);
}

static void updateBuzzer() {
  if (s_phase != AlarmPhase::ACTIVE) {
    setBuzzer(false);
    return;
  }
  uint32_t period = Cfg::BUZZER_BEEP_ON_MS + Cfg::BUZZER_BEEP_OFF_MS;
  uint32_t pos = millis() % period;
  setBuzzer(pos < Cfg::BUZZER_BEEP_ON_MS);
}

static void updateLed() {
  // BUG 3 FIX: ACTIVE and RESET_COOLDOWN take priority over sensor fault.
  // A DHT dropout during an active fire must not hide the alarm LED pattern.
  if (s_phase == AlarmPhase::ACTIVE) {
    LEDManager::setMode(LEDMode::ALARM_ACTIVE);
    return;
  }
  if (s_phase == AlarmPhase::RESET_COOLDOWN) {
    LEDManager::setMode(LEDMode::RESET_BREATHING);
    return;
  }
  if (SensorManager::isSensorFault()) {
    LEDManager::setMode(LEDMode::SENSOR_ERROR);
    return;
  }
  SystemStatus st = SharedState::snapshot();
  if (st.state == SystemState::ONLINE || st.state == SystemState::OFFLINE) {
    if (st.wifiConnected && st.firebaseAuthFailed) {
      LEDManager::setMode(LEDMode::AUTH_ERROR);  // slow breathing red on last 2 LEDs
    } else if (st.wifiConnected && st.firebaseDbFailed) {
      LEDManager::setMode(LEDMode::DB_ERROR);    // fast blinking red on last 2 LEDs
    } else {
      LEDManager::setMode(st.offlineMode ? LEDMode::OFFLINE_STABLE : LEDMode::ONLINE_STABLE);
    }
  }
}

namespace AlarmManager {

void begin() {
  // Active-HIGH button: use INPUT_PULLDOWN so the pin sits at LOW when idle.
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  LOG_I(TAG, "AlarmManager initialized");
}

void tick() {
  if (detectButtonPressEdge()) {
    triggerReset("button", "Alarm Reset: button pressed, entering 20s cooldown.");
  }

  // Remote reset: dashboard set alert=0 while alarm was ringing.
  // BUG 1 FIX: check phase FIRST, then consume the flag — otherwise consumeRemoteReset()
  // clears the flag even when we can't act on it (e.g. already in RESET_COOLDOWN).
  if (s_phase == AlarmPhase::ACTIVE && SharedState::consumeRemoteReset()) {
    triggerReset("remote dashboard", "Alarm Reset: stopped remotely via dashboard, entering 20s cooldown.");
  }

  if (s_phase == AlarmPhase::RESET_COOLDOWN) {
    if (millis() - s_resetStartedMs >= Cfg::RESET_IGNORE_WINDOW_MS) {
      s_phase = AlarmPhase::STABLE;
      LOG_I(TAG, "Reset 20s cooldown completed - sending 'ok' alert status");
      uint32_t epoch = nowEpoch();
      queueFirebaseAlert(false, AlertType::NONE, epoch);
      pushTelegramMessage("System Restored: Status OK.");
      SharedState::setAlert(false, AlertType::NONE, epoch);
    }
  } else {
    AlertType candidate = evaluateCandidateAlertType();
    if (candidate != AlertType::NONE) {
      bool isNewAlert   = (s_phase == AlarmPhase::STABLE);
      bool isEscalation = (s_phase == AlarmPhase::ACTIVE) &&
                           (alertTypePriority(candidate) > alertTypePriority(s_alertType));

      if (isNewAlert || isEscalation) {
        // Notify once on trigger or priority escalation — no keepalive repeat.
        s_phase = AlarmPhase::ACTIVE;
        s_alertType = candidate;
        notify();
      }
      // No else: while alarm stays at the same priority, buzzer+LED handle it locally.
      // Firebase and Telegram are NOT spammed while the alarm is ringing.
    } else if (s_phase == AlarmPhase::ACTIVE) {
      // All sensors returned to safe levels — auto-reset.
      triggerReset("sensors cleared", "Alarm Auto-Reset: all sensor readings returned to safe levels.");
    }
  }

  updateBuzzer();
  updateLed();
}

bool isAlarmActive() { return s_phase == AlarmPhase::ACTIVE; }
AlertType currentAlertType() { return s_alertType; }

}  // namespace AlarmManager
