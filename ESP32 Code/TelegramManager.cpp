#include "TelegramManager.h"
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include "Utils.h"

static const char *TAG = "Telegram";

static WiFiClientSecure s_secureClient;
static UniversalTelegramBot *s_bot = nullptr;
static uint32_t s_lastSendMs = 0;

namespace TelegramManager {

void begin() {
  s_secureClient.setInsecure();
  if (s_bot) {
    delete s_bot;
    s_bot = nullptr;
  }
  if (g_config.telegram.botToken.length() > 0) {
    s_bot = new UniversalTelegramBot(g_config.telegram.botToken, s_secureClient);
    if (!s_bot) {
      // BUG-C FIX: allocation can fail under heap fragmentation after many reconnects.
      // Log clearly so the failure is visible on Serial rather than silently disabling Telegram.
      LOG_E(TAG, "OOM: failed to allocate UniversalTelegramBot — Telegram disabled until next reconnect");
    }
  }
}

void sendToAll(const String &text) {
  if (!s_bot) return;
  for (const auto &userId : g_config.telegram.userIds) {
    bool ok = s_bot->sendMessage(userId, text, "");
    if (!ok) {
      LOG_E(TAG, "Failed to deliver message to chat id %s", userId.c_str());
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

void tick() {
  if (!s_bot || !g_config.telegram.isValid()) return;
  if (!everyMs(s_lastSendMs, Cfg::TELEGRAM_QUEUE_PERIOD_MS)) return;

  TelegramMessageItem item;
  if (xQueueReceive(g_telegramQueue, &item, 0) == pdTRUE) {
    sendToAll(String(item.text));
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

}  // namespace TelegramManager
