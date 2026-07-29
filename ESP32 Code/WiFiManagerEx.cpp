#include "WiFiManagerEx.h"
#include <WiFi.h>
#include <time.h>
#include "Utils.h"

static const char *TAG = "WiFi";

static bool s_connected = false;
static size_t s_nextNetworkIndex = 0;
static uint32_t s_lastReconnectAttemptMs = 0;
static uint8_t s_consecutiveFailures = 0;

static bool tryConnectOnce(const WiFiCredential &cred, uint32_t timeoutMs) {
  WiFi.disconnect(true, false);
  vTaskDelay(pdMS_TO_TICKS(100));

  if (cred.password.length() == 0) {
    WiFi.begin(cred.ssid.c_str());
  } else {
    WiFi.begin(cred.ssid.c_str(), cred.password.c_str());
  }

  uint32_t start = millis();
  while (millis() - start < timeoutMs) {
    if (WiFi.status() == WL_CONNECTED) return true;
    vTaskDelay(pdMS_TO_TICKS(200));
  }
  return false;
}

static void onConnected() {
  s_connected = true;
  s_consecutiveFailures = 0;
  configTime(19800, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "IST-5:30", 1);
  tzset();
  SharedState::setWifiStatus(true, false, (int8_t)WiFi.RSSI(), WiFi.SSID());
  LOG_I(TAG, "Connected to AP: %s (RSSI %d dBm) [Timezone: IST +5:30]", WiFi.SSID().c_str(), WiFi.RSSI());
}

static uint32_t currentBackoffMs() {
  uint8_t capped = s_consecutiveFailures > 6 ? 6 : s_consecutiveFailures;
  uint32_t backoff = Cfg::WIFI_RECONNECT_PERIOD_MS * (1UL << capped);
  return backoff > Cfg::WIFI_RECONNECT_MAX_BACKOFF_MS ? Cfg::WIFI_RECONNECT_MAX_BACKOFF_MS : backoff;
}

static void attemptSingleBackgroundReconnect() {
  if (!g_config.hasWifi()) return;
  const WiFiCredential &cred = g_config.wifiList[s_nextNetworkIndex];
  s_nextNetworkIndex = (s_nextNetworkIndex + 1) % g_config.wifiList.size();
  LOG_I(TAG, "Background retry (failures=%d): '%s'", s_consecutiveFailures, cred.ssid.c_str());
  if (tryConnectOnce(cred, 5000)) {
    onConnected();
  } else {
    if (s_consecutiveFailures < 255) s_consecutiveFailures++;
  }
}

namespace WiFiManagerEx {

void begin() {
  WiFi.mode(WIFI_STA);
  WiFi.persistent(false);
  WiFi.setAutoReconnect(false);
}

bool connectWithFullRetrySequence() {
  if (!g_config.hasWifi()) return false;

  for (uint8_t round = 0; round < Cfg::WIFI_MAX_ROUNDS; round++) {
    for (size_t i = 0; i < g_config.wifiList.size(); i++) {
      const WiFiCredential &cred = g_config.wifiList[i];
      LOG_I(TAG, "Round %d/%d: trying '%s'", round + 1, Cfg::WIFI_MAX_ROUNDS, cred.ssid.c_str());
      if (tryConnectOnce(cred, Cfg::WIFI_CONNECT_TIMEOUT_MS)) {
        onConnected();
        return true;
      }
      vTaskDelay(pdMS_TO_TICKS(Cfg::WIFI_RETRY_STEP_DELAY_MS));
    }
  }

  LOG_W(TAG, "All WiFi rounds exhausted, falling back to offline mode");
  return false;
}

bool waitForInternet(uint32_t timeoutMs) {
  if (WiFi.status() != WL_CONNECTED) return false;

  LOG_I(TAG, "Waiting for internet access & NTP time sync...");
  uint32_t start = millis();

  while (millis() - start < timeoutMs) {
    IPAddress testIp;
    bool dnsOk = (WiFi.hostByName("identitytoolkit.googleapis.com", testIp) == 1) ||
                 (WiFi.hostByName("pool.ntp.org", testIp) == 1);

    time_t now = time(nullptr);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);
    bool ntpOk = (timeinfo.tm_year + 1900) >= 2024;

    if (dnsOk && ntpOk) {
      LOG_I(TAG, "Internet & NTP verified! Time: %04d-%02d-%02d %02d:%02d:%02d",
            timeinfo.tm_year + 1900, timeinfo.tm_mon + 1, timeinfo.tm_mday,
            timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
      return true;
    }

    vTaskDelay(pdMS_TO_TICKS(250));
  }

  IPAddress testIp;
  if (WiFi.hostByName("identitytoolkit.googleapis.com", testIp) == 1) {
    LOG_I(TAG, "DNS resolution confirmed, proceeding to authentication");
    return true;
  }

  LOG_W(TAG, "Internet / NTP check timed out");
  return false;
}

void tick() {
  bool nowConnected = (WiFi.status() == WL_CONNECTED);

  if (s_connected && !nowConnected) {
    s_connected = false;
    SharedState::setWifiStatus(false, true, 0, "");
    LOG_W(TAG, "Connection lost, switching to offline mode");
  }

  if (nowConnected) {
    s_connected = true;
    SharedState::setWifiStatus(true, false, (int8_t)WiFi.RSSI(), WiFi.SSID());
    return;
  }

  if (everyMs(s_lastReconnectAttemptMs, currentBackoffMs())) {
    attemptSingleBackgroundReconnect();
  }
}

bool isConnected() { return s_connected; }
int8_t getRssi() { return s_connected ? (int8_t)WiFi.RSSI() : 0; }
String getCurrentSsid() { return s_connected ? WiFi.SSID() : String(""); }

}  // namespace WiFiManagerEx
