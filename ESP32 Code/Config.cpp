#include "Config.h"

DeviceConfig g_config;

const char *alertTypeToString(AlertType type) {
  switch (type) {
    case AlertType::FIRE: return "fire";
    case AlertType::SMOKE: return "smoke";
    case AlertType::HIGH_TEMP: return "hightemp";
    case AlertType::RESET_EVENT: return "reset";
    default: return "ok";
  }
}

uint8_t alertTypePriority(AlertType type) {
  switch (type) {
    case AlertType::FIRE: return 3;
    case AlertType::SMOKE: return 2;
    case AlertType::HIGH_TEMP: return 1;
    default: return 0;
  }
}

bool FirebaseCredentials::isValid() const {
  return apiKey.length() > 0 && databaseURL.length() > 0 &&
         projectId.length() > 0 && email.length() > 0 && password.length() > 0;
}

bool TelegramConfig::isValid() const {
  return botToken.length() > 0 && !userIds.empty();
}

bool DeviceConfig::hasWifi() const { return !wifiList.empty(); }
