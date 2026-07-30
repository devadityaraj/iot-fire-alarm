#include "Storage.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include "Utils.h"

static const char *TAG = "Storage";
static Preferences s_prefs;

namespace Storage {

void begin() {
  s_prefs.begin(Cfg::NVS_NAMESPACE, false);
}

bool hasConfiguration() {
  bool valid = s_prefs.getBool("valid", false);
  return valid;
}

bool loadConfig(DeviceConfig &cfg) {
  if (!hasConfiguration()) return false;

  cfg.wifiList.clear();
  cfg.telegram.userIds.clear();

  String wifiJson = s_prefs.getString("wifiJson", "[]");
  JsonDocument wifiDoc;
  if (deserializeJson(wifiDoc, wifiJson) == DeserializationError::Ok) {
    for (JsonObject item : wifiDoc.as<JsonArray>()) {
      WiFiCredential cred;
      cred.ssid     = item["ssid"].as<String>();
      cred.password = item["pass"].as<String>();  
      if (cred.ssid.length() > 0) {               
        cfg.wifiList.push_back(cred);
      } else {
        LOG_W(TAG, "Skipping WiFi entry with empty SSID");
      }
    }
  }

  String fbJson = s_prefs.getString("fbJson", "{}");
  JsonDocument fbDoc;
  if (deserializeJson(fbDoc, fbJson) == DeserializationError::Ok) {
    cfg.firebase.apiKey = fbDoc["apiKey"].as<String>();
    cfg.firebase.authDomain = fbDoc["authDomain"].as<String>();
    cfg.firebase.databaseURL = fbDoc["databaseURL"].as<String>();
    cfg.firebase.projectId = fbDoc["projectId"].as<String>();
    cfg.firebase.storageBucket = fbDoc["storageBucket"].as<String>();
    cfg.firebase.messagingSenderId = fbDoc["messagingSenderId"].as<String>();
    cfg.firebase.appId = fbDoc["appId"].as<String>();
    cfg.firebase.email = fbDoc["email"].as<String>();
    cfg.firebase.password = fbDoc["password"].as<String>();
  }

  String tgJson = s_prefs.getString("tgJson", "{}");
  JsonDocument tgDoc;
  if (deserializeJson(tgDoc, tgJson) == DeserializationError::Ok) {
    cfg.telegram.botToken = tgDoc["botToken"].as<String>();
    if (tgDoc["userIds"].is<JsonArray>()) {
      for (JsonVariant v : tgDoc["userIds"].as<JsonArray>()) {
        cfg.telegram.userIds.push_back(v.as<String>());
      }
    }
  }

  LOG_I(TAG, "Configuration loaded: %d wifi networks, %d telegram users",
        cfg.wifiList.size(), cfg.telegram.userIds.size());
  return true;
}

bool saveConfig(const DeviceConfig &cfg) {
  JsonDocument wifiDoc;
  JsonArray wifiArr = wifiDoc.to<JsonArray>();
  for (const auto &cred : cfg.wifiList) {
    JsonObject item = wifiArr.add<JsonObject>();
    item["ssid"] = cred.ssid;
    item["pass"] = cred.password;
  }
  String wifiJson;
  serializeJson(wifiDoc, wifiJson);

  JsonDocument fbDoc;
  fbDoc["apiKey"] = cfg.firebase.apiKey;
  fbDoc["authDomain"] = cfg.firebase.authDomain;
  fbDoc["databaseURL"] = cfg.firebase.databaseURL;
  fbDoc["projectId"] = cfg.firebase.projectId;
  fbDoc["storageBucket"] = cfg.firebase.storageBucket;
  fbDoc["messagingSenderId"] = cfg.firebase.messagingSenderId;
  fbDoc["appId"] = cfg.firebase.appId;
  fbDoc["email"] = cfg.firebase.email;
  fbDoc["password"] = cfg.firebase.password;
  String fbJson;
  serializeJson(fbDoc, fbJson);

  JsonDocument tgDoc;
  tgDoc["botToken"] = cfg.telegram.botToken;
  JsonArray idsArr = tgDoc["userIds"].to<JsonArray>();
  for (const auto &id : cfg.telegram.userIds) {
    idsArr.add(id);
  }
  String tgJson;
  serializeJson(tgDoc, tgJson);

  bool ok = true;
  ok &= s_prefs.putString("wifiJson", wifiJson) > 0 || wifiJson == "[]";
  ok &= s_prefs.putString("fbJson", fbJson) > 0;
  ok &= s_prefs.putString("tgJson", tgJson) > 0;

  if (ok) {
    ok &= s_prefs.putBool("valid", true);
  } else {
    LOG_E(TAG, "One or more NVS writes failed - config NOT marked valid to avoid partial load on reboot");
  }

  LOG_I(TAG, "Configuration saved (%d bytes wifi, %d bytes fb, %d bytes tg)",
        wifiJson.length(), fbJson.length(), tgJson.length());
  return ok;
}

void eraseAll() {
  s_prefs.clear();
  LOG_I(TAG, "All configuration erased");
}

}  
