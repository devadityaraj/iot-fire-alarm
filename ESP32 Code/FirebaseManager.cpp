#include "FirebaseManager.h"
#include "SensorManager.h"
#include <Firebase_ESP_Client.h>
#include <addons/RTDBHelper.h>
#include <math.h>
#include "Utils.h"

static const char *TAG = "Firebase";

static FirebaseData s_fbdo;
static FirebaseAuth s_fbAuth;
static FirebaseConfig s_fbConfig;

static bool s_initialized = false;
static bool s_authed = false;
static bool s_authBlocked = false;
static uint32_t s_blockedUntilMs = 0;

static float s_lastSentTemp = NAN;
static float s_lastSentHumidity = NAN;
static int s_lastSentSmoke = -999;
static uint32_t s_lastTempUploadMs = 0;
static uint32_t s_last5sUploadMs = 0;
static uint32_t s_lastReadyCheckMs = 0;
static bool s_dbError = false;

// Remote reset detection: poll /device/alert to detect a 1→0 transition from the dashboard
static int  s_lastRemoteAlertValue = -1;  // -1 = unknown (not yet polled)
static uint32_t s_lastAlertPollMs = 0;
static FirebaseData s_pollFbdo;           // separate FirebaseData object to avoid clobbering uploads

static void myTokenStatusCallback(TokenInfo info) {
  if (info.status == token_status_error) {
    LOG_E(TAG, "Token error: code: %d, message: %s", info.error.code, info.error.message.c_str());
    String msg = String(info.error.message.c_str());
    if (info.error.code == 400 || msg.indexOf("TOO_MANY_ATTEMPTS") >= 0 ||
        msg.indexOf("INVALID_PASSWORD") >= 0 ||
        msg.indexOf("EMAIL_NOT_FOUND") >= 0 ||
        msg.indexOf("bad request") >= 0) {
      LOG_W(TAG, "Firebase Auth rate-limited or bad credentials. Pausing auth attempts for 5 minutes.");
      s_authBlocked = true;
      s_blockedUntilMs = millis() + 300000;  // 5 minute cooldown
      s_authed = false;
      SharedState::setFirebaseAuthed(false);
      SharedState::setFirebaseError(true, s_dbError);
    }
  } else if (info.status == token_status_ready) {
    LOG_I(TAG, "Firebase Token Ready");
    s_authBlocked = false;
    s_authed = true;
    SharedState::setFirebaseAuthed(true);
    SharedState::setFirebaseError(false, s_dbError);
  }
}

static bool valueChanged(float a, float b) {
  if (isnan(a) != isnan(b)) return true;
  if (isnan(a) && isnan(b)) return false;
  return fabsf(a - b) > 0.05f;
}

static void uploadNode(FirebaseJson &json) {
  bool ok = Firebase.RTDB.updateNode(&s_fbdo, "/device", &json);
  if (!ok) {
    LOG_E(TAG, "RTDB update failed: %s", s_fbdo.errorReason().c_str());
    if (!s_dbError) {
      s_dbError = true;
      SharedState::setFirebaseError(false, true);
    }
  } else if (s_dbError) {
    s_dbError = false;
    SharedState::setFirebaseError(false, false);
  }
}

static void uploadTelemetry() {
  SystemStatus st = SharedState::snapshot();
  FirebaseJson json;
  json.set("temperature", isnan(st.temperature) ? 0.0f : st.temperature);
  json.set("humidity", isnan(st.humidity) ? 0.0f : st.humidity);
  json.set("smoke", st.smokeRaw);
  uploadNode(json);
  s_lastSentTemp = st.temperature;
  s_lastSentHumidity = st.humidity;
  s_lastSentSmoke = st.smokeRaw;
}

static String formatTimestampIST(uint32_t epochSec) {
  if (epochSec == 0) return String("N/A");
  time_t t = (time_t)epochSec;
  struct tm tm_info;
  localtime_r(&t, &tm_info);
  char buf[32];
  snprintf(buf, sizeof(buf), "%02d:%02d-%02d%02d%04d",
           tm_info.tm_hour, tm_info.tm_min,
           tm_info.tm_mday, tm_info.tm_mon + 1, tm_info.tm_year + 1900);
  return String(buf);
}

static void uploadAlertItem(const FirebaseUpdateItem &item) {
  FirebaseJson json;
  json.set("temperature", isnan(item.temperature) ? 0.0f : item.temperature);
  json.set("humidity", isnan(item.humidity) ? 0.0f : item.humidity);
  json.set("smoke", item.smokeRaw);
  json.set("alert", item.alertActive ? 1 : 0);
  json.set("alerttype", alertTypeToString(item.alertType));
  if (item.epochTime > 0) {
    json.set("lastTrigger", formatTimestampIST(item.epochTime));
  }
  uploadNode(json);
  s_lastSentTemp = item.temperature;
  s_lastSentHumidity = item.humidity;
  s_lastSentSmoke = item.smokeRaw;

  // BUG 4 FIX: When we write alert=0 ourselves, sync the poll's last-known value
  // so the 3s poller doesn't see a self-caused 1->0 and trigger a spurious remote reset
  // on the next alarm cycle.
  if (!item.alertActive) {
    s_lastRemoteAlertValue = 0;
  }
}

namespace FirebaseManager {

void begin() {
  if (!g_config.firebase.isValid()) {
    LOG_W(TAG, "Firebase not configured or incomplete, skipping initialization");
    s_initialized = false;
    s_authed = false;
    SharedState::setFirebaseAuthed(false);
    return;
  }

  s_fbConfig.api_key = g_config.firebase.apiKey;
  s_fbConfig.database_url = g_config.firebase.databaseURL;
  s_fbAuth.user.email = g_config.firebase.email;
  s_fbAuth.user.password = g_config.firebase.password;
  s_fbConfig.token_status_callback = myTokenStatusCallback;
  s_fbConfig.max_token_generation_retry = 2;

  // Let WiFiManagerEx handle WiFi reconnects to avoid blocking loops in Firebase library
  Firebase.reconnectWiFi(false);
  Firebase.begin(&s_fbConfig, &s_fbAuth);
  s_initialized = true;
  LOG_I(TAG, "Firebase initialized");
}

bool authenticate() {
  if (!s_initialized || !g_config.firebase.isValid()) {
    s_authed = false;
    SharedState::setFirebaseAuthed(false);
    SharedState::setFirebaseError(true, s_dbError);
    return false;
  }

  if (SensorManager::isSensorFault()) {
    LOG_W(TAG, "Sensor fault active (DHT & Smoke missing). Firebase auth deferred.");
    s_authed = false;
    SharedState::setFirebaseAuthed(false);
    return false;
  }

  if (s_authBlocked) {
    LOG_W(TAG, "Firebase auth is currently in 5-minute cooldown. Skipping.");
    return false;
  }

  uint32_t start = millis();
  while (!Firebase.ready() && millis() - start < 3000) {
    vTaskDelay(pdMS_TO_TICKS(200));
    if (s_authBlocked) break;
  }

  s_authed = Firebase.ready();
  SharedState::setFirebaseAuthed(s_authed);
  SharedState::setFirebaseError(!s_authed, s_dbError);

  if (s_authed) {
    LOG_I(TAG, "Firebase authentication successful");
    uint32_t now = millis();
    s_lastTempUploadMs = now;
    s_last5sUploadMs = now;
  } else {
    LOG_W(TAG, "Firebase auth pending or failed.");
  }
  return s_authed;
}

void tick() {
  if (!s_initialized) return;

  if (SensorManager::isSensorFault()) {
    return;  // Do not connect or send data to Firebase while sensor fault is active
  }

  uint32_t now = millis();

  // If auth is blocked due to 400 / TOO_MANY_ATTEMPTS, enforce mandatory 5-min cooldown
  if (s_authBlocked) {
    if (now < s_blockedUntilMs) {
      return;  // Completely skip checking Firebase.ready() to stop loop
    }
    LOG_I(TAG, "5-minute Firebase auth cooldown finished. Resetting auth block...");
    s_authBlocked = false;
  }

  // Rate-limit status polling to once every 2 seconds
  if (now - s_lastReadyCheckMs < 2000) {
    if (!s_authed) return;
  } else {
    s_lastReadyCheckMs = now;
    s_authed = Firebase.ready();
    SharedState::setFirebaseAuthed(s_authed);
    if (!s_authed && !s_authBlocked) {
      SharedState::setFirebaseError(true, s_dbError);
    }
  }

  if (!s_authed) return;

  FirebaseUpdateItem alertItem;
  while (xQueueReceive(g_firebaseQueue, &alertItem, 0) == pdTRUE) {
    uploadAlertItem(alertItem);
    vTaskDelay(pdMS_TO_TICKS(10));  // Yield to give CPU time to system/idle tasks
  }

  // Poll /device/alert every 3 s so we can detect a remote 1→0 reset from the dashboard.
  // Only act when the alarm is locally active to avoid spurious resets on normal startup.
  if (everyMs(s_lastAlertPollMs, Cfg::FIREBASE_ALERT_POLL_MS)) {
    if (Firebase.RTDB.getInt(&s_pollFbdo, "/device/alert")) {
      int remoteAlert = s_pollFbdo.intData();
      bool alarmActive = SharedState::snapshot().alertActive;
      if (s_lastRemoteAlertValue == 1 && remoteAlert == 0 && alarmActive) {
        LOG_I(TAG, "Remote dashboard set alert=0 while alarm active — triggering remote reset");
        SharedState::setRemoteResetRequested(true);
      }
      s_lastRemoteAlertValue = remoteAlert;
    } else {
      LOG_W(TAG, "Alert poll failed: %s", s_pollFbdo.errorReason().c_str());
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }

  // Temperature: upload every 5s, only if value changed by > 0.05 °C
  if (everyMs(s_lastTempUploadMs, Cfg::FIREBASE_TEMP_UPLOAD_MS)) {
    SystemStatus st = SharedState::snapshot();
    if (valueChanged(st.temperature, s_lastSentTemp)) {
      FirebaseJson json;
      json.set("temperature", isnan(st.temperature) ? 0.0f : st.temperature);
      uploadNode(json);
      s_lastSentTemp = st.temperature;
    }
  }

  // Humidity + Smoke: upload every 5s; smoke only if changed by >= 50 ADC counts
  if (everyMs(s_last5sUploadMs, Cfg::FIREBASE_HUM_UPLOAD_MS)) {
    SystemStatus st = SharedState::snapshot();
    FirebaseJson json;
    json.set("humidity", isnan(st.humidity) ? 0.0f : st.humidity);

    bool sendSmoke = (s_lastSentSmoke == -999) || (abs(st.smokeRaw - s_lastSentSmoke) >= 50);
    if (sendSmoke) {
      json.set("smoke", st.smokeRaw);
      s_lastSentSmoke = st.smokeRaw;
    }
    uploadNode(json);
    s_lastSentHumidity = st.humidity;
  }
}

bool isAuthenticated() { return s_authed; }

}  // namespace FirebaseManager
