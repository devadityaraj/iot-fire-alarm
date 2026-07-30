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

// Returns true if setup mode should be entered.
// If the button is held, shows amber immediately during the hold window
// and drives LEDs directly (appTask does not exist yet at this point).
static bool shouldEnterSetupMode() {
  pinMode(BUTTON_PIN, INPUT_PULLDOWN);
  vTaskDelay(pdMS_TO_TICKS(50));  // let pull-down settle

  if (!Storage::hasConfiguration()) return true;  // first boot

  if (digitalRead(BUTTON_PIN) != HIGH) return false;  // not pressed

  // Button detected — show amber right away so user gets visual feedback.
  LEDManager::setMode(LEDMode::SETUP_AMBER);

  uint32_t holdStart = millis();
  uint32_t lastLowMs = 0;
  const uint32_t GLITCH_TOLERANCE_MS = 30;

  while (millis() - holdStart < Cfg::SETUP_BUTTON_HOLD_MS) {
    bool pressed = (digitalRead(BUTTON_PIN) == HIGH);
    if (!pressed) {
      if (lastLowMs == 0) lastLowMs = millis();
      if (millis() - lastLowMs >= GLITCH_TOLERANCE_MS) return false;  // real release
    } else {
      lastLowMs = 0;
    }
    // appTask doesn't exist yet — drive LEDs here directly.
    LEDManager::tick();
    vTaskDelay(pdMS_TO_TICKS(5));
  }
  return true;  // held for full window
}

// LED sequence:
//   first boot  → rainbow 2s → amber (AP running)
//   button hold → amber during hold (driven in shouldEnterSetupMode)
//                 → rainbow 2s → amber (AP running)
// In both cases appTask doesn't run here, so LEDManager::tick() is called
// explicitly in every loop to push frames to the strip.
static void runSetupMode() {
  LOG_I(TAG, "Entering setup mode");
  SharedState::setState(SystemState::SETUP_MODE);

  // Rainbow phase — plays for both first-boot and button-triggered paths.
  LEDManager::setMode(LEDMode::SETUP_RAINBOW);
  uint32_t start = millis();
  while (millis() - start < Cfg::SETUP_RAINBOW_MS) {
    LEDManager::tick();
    vTaskDelay(pdMS_TO_TICKS(20));
  }

  // Amber: AP is now up, user can release the button and configure.
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
    // Keep appTask suspended for the duration of setup so it cannot
    // steal button edges or overwrite the setup-mode LED state.
    if (s_appTaskHandle) vTaskSuspend(s_appTaskHandle);
    runSetupMode();
    // runSetupMode() never returns (it reboots internally), but be tidy:
    return;
  }

  // Normal boot — start the app task now that setup mode was not requested.
  xTaskCreatePinnedToCore(appTask, "AppTask", 8192, nullptr, 2, &s_appTaskHandle, 1);

  SharedState::setState(SystemState::CONNECT_WIFI);
  WiFiManagerEx::begin();
  bool connected = WiFiManagerEx::connectWithFullRetrySequence();

  if (connected) {
    LEDManager::setMode(LEDMode::CONNECTED_BLINK);
    while (!LEDManager::isConnectBlinkDone()) vTaskDelay(pdMS_TO_TICKS(20));

    // Wait for actual Internet access & NTP time sync before calling Firebase Auth
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
  vTaskDelay(pdMS_TO_TICKS(1000));  // 1-second power-on stabilization delay

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

  // appTask is created inside netTask *after* the setup-mode check so that
  // AlarmManager::tick() cannot consume the button edge or override the LED
  // during the hold-detection window. Only netTask is spawned here.
  // Pinned to Core 1 so Core 0 remains free for ESP32 WiFi driver & IDLE0 task.
  xTaskCreatePinnedToCore(netTask, "NetTask", 16384, nullptr, 1, nullptr, 1);

  LOG_I(TAG, "Firmware %s started", Cfg::FIRMWARE_VERSION);
}

}  // namespace TaskManager
