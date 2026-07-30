#include "TaskManager.h"
#include "Config.h"
#include "Pins.h"
#include "Utils.h"
#include "Storage.h"
#include "SensorManager.h"
#include "AlarmManager.h"
#include "LEDManager.h"
#include "WiFiManagerEx.h"
#include "FirebaseManager.h"
#include "TelegramManager.h"
#include "WebPortal.h"
#include <esp_task_wdt.h>

static const char *TAG = "TaskManager";

static bool shouldEnterSetupMode() {
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  vTaskDelay(pdMS_TO_TICKS(50));  

  if (!Storage::hasConfiguration()) return true;  

  if (digitalRead(BUTTON_PIN) != HIGH) return false;  

  LEDManager::setMode(LEDMode::SETUP_AMBER);

  uint32_t holdStart = millis();
  uint32_t lastLowMs = 0;
  const uint32_t GLITCH_TOLERANCE_MS = 30;

  while (millis() - holdStart < Cfg::SETUP_BUTTON_HOLD_MS) {
    bool pressed = (digitalRead(BUTTON_PIN) == HIGH);
    if (!pressed) {
      if (lastLowMs == 0) lastLowMs = millis();
      if (millis() - lastLowMs >= GLITCH_TOLERANCE_MS) return false;  
    } else {
      lastLowMs = 0;
    }
    
    LEDManager::tick();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
  return true;  
}

static void runSetupMode() {
  LOG_I(TAG, "Entering setup mode");
  SharedState::setState(SystemState::SETUP_MODE);

  LEDManager::setMode(LEDMode::SETUP_RAINBOW);
  uint32_t start = millis();
  while (millis() - start < Cfg::SETUP_RAINBOW_MS) {
    LEDManager::tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  LEDManager::setMode(LEDMode::SETUP_AMBER);
  WebPortal::begin();

  for (;;) {
    LEDManager::tick();
    WebPortal::tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

static TaskHandle_t s_appTaskHandle = nullptr;

static void appTask(void *pvParameters) {
  (void)pvParameters;
  uint32_t lastSensor = 0, lastAlarm = 0, lastLed = 0;

  for (;;) {
    if (everyMs(lastSensor, Cfg::SENSOR_TASK_PERIOD_MS)) SensorManager::tick();
    if (everyMs(lastAlarm, Cfg::BUTTON_TASK_PERIOD_MS)) AlarmManager::tick();
    if (everyMs(lastLed, Cfg::LED_TASK_PERIOD_MS)) LEDManager::tick();
    vTaskDelay(pdMS_TO_TICKS(2));
  }
}

static void netTask(void *pvParameters) {
  (void)pvParameters;

  SharedState::setState(SystemState::BOOT);
  LEDManager::setMode(LEDMode::BOOT_CONNECTING);

  if (shouldEnterSetupMode()) {

    if (s_appTaskHandle) vTaskSuspend(s_appTaskHandle);
    runSetupMode();
    
    return;
  }

  xTaskCreatePinnedToCore(appTask, "AppTask", 8192, nullptr, 2, &s_appTaskHandle, 1);

  SharedState::setState(SystemState::CONNECT_WIFI);
  WiFiManagerEx::begin();
  bool connected = WiFiManagerEx::connectWithFullRetrySequence();

  if (connected) {
    LEDManager::setMode(LEDMode::CONNECTED_BLINK);
    while (!LEDManager::isConnectBlinkDone()) vTaskDelay(pdMS_TO_TICKS(20));

    if (WiFiManagerEx::waitForInternet(8000)) {
      SharedState::setState(SystemState::AUTH_FIREBASE);
      FirebaseManager::begin();
      FirebaseManager::authenticate();
      TelegramManager::begin();
    } else {
      LOG_W(TAG, "Wi-Fi connected but internet check timed out. Deferred auth.");
    }
  }

  SharedState::setState(connected ? SystemState::ONLINE : SystemState::OFFLINE);
  LEDManager::setMode(connected ? LEDMode::ONLINE_STABLE : LEDMode::OFFLINE_STABLE);

  bool wasOnline = connected;
  uint32_t bootMs = millis();
  uint32_t lastStatusTick = 0;

  for (;;) {
    WiFiManagerEx::tick();
    bool nowConnected = WiFiManagerEx::isConnected();

    if (nowConnected && !wasOnline) {
      LOG_I(TAG, "Back online, verifying internet connection...");
      if (WiFiManagerEx::waitForInternet(5000)) {
        SharedState::setState(SystemState::AUTH_FIREBASE);
        if (!FirebaseManager::isAuthenticated()) FirebaseManager::authenticate();
        TelegramManager::begin();
        SharedState::setState(SystemState::ONLINE);
        pushTelegramMessage("Connection Restored: device is back online.");
      }
    } else if (!nowConnected && wasOnline) {
      LOG_W(TAG, "Connection lost, falling back to offline mode");
      SharedState::setState(SystemState::OFFLINE);
      pushTelegramMessage("Connection Lost: device switched to offline mode.");
    }
    wasOnline = nowConnected;

    if (nowConnected) {
      FirebaseManager::tick();
      TelegramManager::tick();
    }

    if (everyMs(lastStatusTick, Cfg::STATUS_TASK_PERIOD_MS)) {
      SharedState::setUptime((millis() - bootMs) / 1000);
    }

    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

namespace TaskManager {

void begin() {
  vTaskDelay(pdMS_TO_TICKS(1000));  

  esp_task_wdt_config_t twdt_cfg = {
      .timeout_ms = 15000,
      .idle_core_mask = (1 << 0) | (1 << 1),
      .trigger_panic = true};
  esp_task_wdt_reconfigure(&twdt_cfg);

  SharedState::init();
  Storage::begin();
  Storage::loadConfig(g_config);

  SensorManager::begin();
  AlarmManager::begin();
  LEDManager::begin();

  xTaskCreatePinnedToCore(netTask, "NetTask", 16384, nullptr, 1, nullptr, 1);

  LOG_I(TAG, "Firmware %s started", Cfg::FIRMWARE_VERSION);
}

}  
