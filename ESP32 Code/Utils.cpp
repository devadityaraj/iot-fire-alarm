#include "Utils.h"
#include <time.h>

static SystemStatus s_status;
static SemaphoreHandle_t s_statusMutex = nullptr;

QueueHandle_t g_firebaseQueue = nullptr;
QueueHandle_t g_telegramQueue = nullptr;

namespace SharedState {

void init() {
  s_statusMutex = xSemaphoreCreateMutex();
  g_firebaseQueue = xQueueCreate(16, sizeof(FirebaseUpdateItem));
  g_telegramQueue = xQueueCreate(16, sizeof(TelegramMessageItem));
}

SystemStatus snapshot() {
  SystemStatus copy;
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    copy = s_status;
    xSemaphoreGive(s_statusMutex);
  }
  return copy;
}

void setState(SystemState s) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.state = s;
    xSemaphoreGive(s_statusMutex);
  }
}

void setWifiStatus(bool connected, bool offline, int8_t rssi, const String &ssid) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.wifiConnected = connected;
    s_status.offlineMode = offline;
    s_status.rssi = rssi;
    s_status.currentSSID = ssid;
    xSemaphoreGive(s_statusMutex);
  }
}

void setFirebaseAuthed(bool authed) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.firebaseAuthed = authed;
    xSemaphoreGive(s_statusMutex);
  }
}

void setFirebaseError(bool authFailed, bool dbFailed) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.firebaseAuthFailed = authFailed;
    s_status.firebaseDbFailed   = dbFailed;
    xSemaphoreGive(s_statusMutex);
  }
}

void setSensorData(float temp, float hum, int smoke, bool flame) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.temperature = temp;
    s_status.humidity = hum;
    s_status.smokeRaw = smoke;
    s_status.flameActive = flame;
    xSemaphoreGive(s_statusMutex);
  }
}

void setAlert(bool active, AlertType type, uint32_t epoch) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.alertActive = active;
    s_status.alertType = type;
    if (epoch > 0) s_status.lastAlertEpoch = epoch;
    xSemaphoreGive(s_statusMutex);
  }
}

void setUptime(uint32_t seconds) {
  if (xSemaphoreTake(s_statusMutex, portMAX_DELAY) == pdTRUE) {
    s_status.uptimeSeconds = seconds;
    xSemaphoreGive(s_statusMutex);
  }
}

}  // namespace SharedState

bool pushFirebaseUpdate(const FirebaseUpdateItem &item) {
  if (!g_firebaseQueue) return false;
  return xQueueSend(g_firebaseQueue, &item, 0) == pdTRUE;
}

bool pushTelegramMessage(const String &text) {
  if (!g_config.telegram.isValid() || !g_telegramQueue) return false;
  TelegramMessageItem item{};
  strncpy(item.text, text.c_str(), sizeof(item.text) - 1);
  return xQueueSend(g_telegramQueue, &item, 0) == pdTRUE;
}

uint32_t nowEpoch() {
  time_t t = time(nullptr);
  return static_cast<uint32_t>(t);
}
