#pragma once
#include <Arduino.h>
#include <vector>

namespace Cfg {
static constexpr uint32_t FLAME_TRIGGER_MS = 300;
static constexpr uint16_t SMOKE_THRESHOLD = 3000;
static constexpr float TEMP_HIGH_THRESHOLD_C = 45.0f;

static constexpr uint32_t SENSOR_TASK_PERIOD_MS = 10;
static constexpr uint32_t LED_TASK_PERIOD_MS = 20;
static constexpr uint32_t BUTTON_TASK_PERIOD_MS = 20;
static constexpr uint32_t ALARM_TASK_PERIOD_MS = 50;
static constexpr uint32_t DHT_READ_PERIOD_MS = 2000;

static constexpr uint32_t FIREBASE_TEMP_UPLOAD_MS    = 5000;   
static constexpr uint32_t FIREBASE_HUM_UPLOAD_MS     = 5000;   
static constexpr uint32_t FIREBASE_ALERT_POLL_MS     = 3000;   

static constexpr uint32_t WIFI_RECONNECT_PERIOD_MS        = 10000;
static constexpr uint32_t WIFI_RECONNECT_MAX_BACKOFF_MS   = 60000;
static constexpr uint32_t TELEGRAM_QUEUE_PERIOD_MS        = 200;
static constexpr uint32_t STATUS_TASK_PERIOD_MS           = 1000;

static constexpr uint32_t RESET_IGNORE_WINDOW_MS = 20000;
static constexpr uint32_t RESET_LED_DURATION_MS = 20000;

static constexpr uint32_t BUTTON_DEBOUNCE_MS = 50;
static constexpr uint32_t SETUP_BUTTON_HOLD_MS = 2000;
static constexpr uint32_t SETUP_RAINBOW_MS = 2000;

static constexpr uint8_t WIFI_MAX_ROUNDS = 3;
static constexpr uint32_t WIFI_CONNECT_TIMEOUT_MS = 8000;
static constexpr uint32_t WIFI_RETRY_STEP_DELAY_MS = 250;

static constexpr uint32_t BUZZER_BEEP_ON_MS = 300;
static constexpr uint32_t BUZZER_BEEP_OFF_MS = 300;

static constexpr uint32_t FIREBASE_TOKEN_CHECK_MS = 5000;

static constexpr uint8_t LED_BRIGHTNESS_HALF = 128;
static constexpr uint8_t LED_BRIGHTNESS_FULL = 255;

static constexpr const char *FIRMWARE_VERSION = "1.0.0";
static constexpr const char *AP_SSID = "Smart IoT Fire Alarm";
static constexpr const char *AP_IP = "192.168.4.1";
static constexpr const char *NVS_NAMESPACE = "firealarm";
static constexpr uint16_t WEB_SERVER_PORT = 80;
}  

enum class SystemState : uint8_t {
  BOOT,
  CHECK_CONFIGURATION,
  SETUP_MODE,
  CONNECT_WIFI,
  AUTH_FIREBASE,
  ONLINE,
  OFFLINE,
  ALARM,
  RESET
};

enum class AlertType : uint8_t { NONE, HIGH_TEMP, SMOKE, FIRE, RESET_EVENT };

enum class LEDMode : uint8_t {
  BOOT_CONNECTING,
  CONNECTED_BLINK,
  ONLINE_STABLE,
  OFFLINE_STABLE,
  ALARM_ACTIVE,
  RESET_BREATHING,
  SETUP_RAINBOW,
  SETUP_AMBER,
  AUTH_ERROR,   
  DB_ERROR,     
  SENSOR_ERROR  
};

const char *alertTypeToString(AlertType type);
uint8_t alertTypePriority(AlertType type);

struct WiFiCredential {
  String ssid;
  String password;
};

struct FirebaseCredentials {
  String apiKey;
  String authDomain;
  String databaseURL;
  String projectId;
  String storageBucket;
  String messagingSenderId;
  String appId;
  String email;
  String password;

  bool isValid() const;
};

struct TelegramConfig {
  String botToken;
  std::vector<String> userIds;

  bool isValid() const;
};

struct DeviceConfig {
  std::vector<WiFiCredential> wifiList;
  FirebaseCredentials firebase;
  TelegramConfig telegram;

  bool hasWifi() const;
};

extern DeviceConfig g_config;
