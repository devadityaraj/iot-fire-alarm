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

static uint32_t s_lastFirebaseNotifyMs = 0;
static uint32_t s_lastTelegramNotifyMs = 0;

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

// New alerts and priority escalations bypass cooldown (meaningful new information);
// while latched at the same priority, notifications repeat only at the cooldown cadence
// so continuous re-triggering of the same channel does not spam Firebase/Telegram.
static void notify(bool bypassFirebaseCooldown, bool bypassTelegramCooldown) {
  uint32_t now = millis();
  uint32_t epoch = nowEpoch();

  bool sendFirebase = bypassFirebaseCooldown ||
                       (now - s_lastFirebaseNotifyMs >= Cfg::FIREBASE_ALERT_COOLDOWN_MS);
  bool sendTelegram = bypassTelegramCooldown ||
                       (now - s_lastTelegramNotifyMs >= Cfg::TELEGRAM_ALERT_COOLDOWN_MS);

  if (sendFirebase) {
    queueFirebaseAlert(true, s_alertType, epoch);
    s_lastFirebaseNotifyMs = now;
  }
  if (sendTelegram) {
    pushTelegramMessage(buildAlertMessage(s_alertType));
    s_lastTelegramNotifyMs = now;
  }

  SharedState::setAlert(true, s_alertType, epoch);
}

static void triggerReset() {
  LOG_I(TAG, "Reset button pressed - entering 20s cooldown window");
  s_phase = AlarmPhase::RESET_COOLDOWN;
  s_alertType = AlertType::NONE;
  s_resetStartedMs = millis();
  setBuzzer(false);
  LEDManager::setMode(LEDMode::RESET_BREATHING);

  pushTelegramMessage("Alarm Reset: button pressed, entering 20s cooldown.");
  SharedState::setAlert(false, AlertType::NONE, nowEpoch());

  s_lastFirebaseNotifyMs = millis();
  s_lastTelegramNotifyMs = millis();
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
  if (SensorManager::isSensorFault()) {
    LEDManager::setMode(LEDMode::SENSOR_ERROR);
    return;
  }
  if (s_phase == AlarmPhase::ACTIVE) {
    LEDManager::setMode(LEDMode::ALARM_ACTIVE);
    return;
  }
  if (s_phase == AlarmPhase::RESET_COOLDOWN) {
    LEDManager::setMode(LEDMode::RESET_BREATHING);
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
    triggerReset();
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
      bool isNewAlert = (s_phase == AlarmPhase::STABLE);
      bool isEscalation = (s_phase == AlarmPhase::ACTIVE) &&
                           (alertTypePriority(candidate) > alertTypePriority(s_alertType));

      if (isNewAlert || isEscalation) {
        s_phase = AlarmPhase::ACTIVE;
        s_alertType = candidate;
        notify(true, true);
      } else if (s_phase == AlarmPhase::ACTIVE) {
        notify(false, false);
      }
    } else if (s_phase == AlarmPhase::ACTIVE) {
      notify(false, false);
    }
  }

  updateBuzzer();
  updateLed();
}

bool isAlarmActive() { return s_phase == AlarmPhase::ACTIVE; }
AlertType currentAlertType() { return s_alertType; }

}  // namespace AlarmManager
