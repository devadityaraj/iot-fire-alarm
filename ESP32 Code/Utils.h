#pragma once
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include "Config.h"

#define LOG_I(tag, fmt, ...) Serial.printf("[I][%s] " fmt "\r\n", tag, ##__VA_ARGS__)
#define LOG_W(tag, fmt, ...) Serial.printf("[W][%s] " fmt "\r\n", tag, ##__VA_ARGS__)
#define LOG_E(tag, fmt, ...) Serial.printf("[E][%s] " fmt "\r\n", tag, ##__VA_ARGS__)

inline bool everyMs(uint32_t &lastRun, uint32_t interval) {
  uint32_t now = millis();
  if (now - lastRun >= interval) {
    lastRun = now;
    return true;
  }
  return false;
}

struct SystemStatus {
  SystemState state = SystemState::BOOT;
  bool wifiConnected = false;
  bool offlineMode = true;
  String currentSSID;
  float temperature = NAN;
  float humidity = NAN;
  int smokeRaw = 0;
  bool flameActive = false;
  bool alertActive = false;
  AlertType alertType = AlertType::NONE;
  uint32_t uptimeSeconds = 0;
  uint32_t lastAlertEpoch = 0;
  bool firebaseAuthed = false;
  bool firebaseAuthFailed = false;  // auth (email/password) failed
  bool firebaseDbFailed   = false;  // last RTDB write failed
  bool remoteResetRequested = false; // set by Firebase when alert 1->0 from dashboard
};

struct FirebaseUpdateItem {
  bool isAlertUpdate;
  float temperature;
  float humidity;
  int smokeRaw;
  bool flameActive;
  bool alertActive;
  AlertType alertType;
  uint32_t epochTime;
};

struct TelegramMessageItem {
  char text[192];
};

namespace SharedState {
void init();
SystemStatus snapshot();
void setState(SystemState s);
void setWifiStatus(bool connected, bool offline, const String &ssid);
void setFirebaseAuthed(bool authed);
void setFirebaseError(bool authFailed, bool dbFailed);
void setSensorData(float temp, float hum, int smoke, bool flame);
void setAlert(bool active, AlertType type, uint32_t epoch);
void setUptime(uint32_t seconds);
void setRemoteResetRequested(bool val);
bool consumeRemoteReset();  // returns true (and clears flag) if a remote reset is pending
}  // namespace SharedState

extern QueueHandle_t g_firebaseQueue;
extern QueueHandle_t g_telegramQueue;

bool pushFirebaseUpdate(const FirebaseUpdateItem &item);
bool pushTelegramMessage(const String &text);

uint32_t nowEpoch();
