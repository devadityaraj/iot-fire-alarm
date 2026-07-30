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

static bool s_lastButtonReading = LOW;   
static bool s_buttonStableState = LOW;
static uint32_t s_lastDebounceMs = 0;

static void setBuzzer(bool on) {
  if (on != s_buzzerOn) {
    digitalWrite(BUZZER_PIN, on ? HIGH : LOW);
    s_buzzerOn = on;
  }
}

static bool detectButtonPressEdge() {
  bool reading = digitalRead(BUTTON_PIN);
  if (reading != s_lastButtonReading) {
    s_lastDebounceMs = millis();
  }
  s_lastButtonReading = reading;

  if ((millis() - s_lastDebounceMs) > Cfg::BUTTON_DEBOUNCE_MS) {
    if (reading != s_buttonStableState) {
      s_buttonStableState = reading;
      if (s_buttonStableState == HIGH) return true;  
    }
  }
  return false;
}

static AlertType evaluateCandidateAlertType() {
  if (SensorManager::isSensorFault()) return AlertType::NONE;  
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
      return "SMOKE ALERT: Smoke is detected, please check.";
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

static void notify() {
  uint32_t epoch = nowEpoch();
  queueFirebaseAlert(true, s_alertType, epoch);
  pushTelegramMessage(buildAlertMessage(s_alertType));
  SharedState::setAlert(true, s_alertType, epoch);
}

static void triggerReset(const char* logReason, const char* telegramMsg) {
  LOG_I(TAG, "Alarm reset [%s]", logReason);
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
      LEDManager::setMode(LEDMode::AUTH_ERROR);  
    } else if (st.wifiConnected && st.firebaseDbFailed) {
      LEDManager::setMode(LEDMode::DB_ERROR);    
    } else {
      LEDManager::setMode(st.offlineMode ? LEDMode::OFFLINE_STABLE : LEDMode::ONLINE_STABLE);
    }
  }
}

namespace AlarmManager {

void begin() {
  
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  LOG_I(TAG, "AlarmManager initialized");
}

void tick() {
  if (detectButtonPressEdge()) {
    triggerReset("button", "Alarm Reset: button pressed, entering 20s cooldown.");
  }

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
  } else if (!SensorManager::isSensorFault()) {
    AlertType candidate = evaluateCandidateAlertType();
    if (candidate != AlertType::NONE) {
      bool isNewAlert   = (s_phase == AlarmPhase::STABLE);
      bool isEscalation = (s_phase == AlarmPhase::ACTIVE) &&
                           (alertTypePriority(candidate) > alertTypePriority(s_alertType));

      if (isNewAlert || isEscalation) {
        
        s_phase = AlarmPhase::ACTIVE;
        s_alertType = candidate;
        notify();
      }

    }
  }

  updateBuzzer();
  updateLed();
}

bool isAlarmActive() { return s_phase == AlarmPhase::ACTIVE; }
AlertType currentAlertType() { return s_alertType; }

}  
