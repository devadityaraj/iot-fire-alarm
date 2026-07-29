#include "WebPortal.h"
#include <ESPAsyncWebServer.h>
#include <AsyncJson.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "Config.h"
#include "Storage.h"
#include "Utils.h"

static const char *TAG = "WebPortal";
static AsyncWebServer s_server(Cfg::WEB_SERVER_PORT);
static bool s_rebootPending = false;
static uint32_t s_rebootAtMs = 0;

static void scheduleReboot(uint32_t delayMs) {
  s_rebootPending = true;
  s_rebootAtMs = millis() + delayMs;
}

static const char INDEX_HTML[] PROGMEM = R"HTMLPAGE(<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1.0">
<title>Smart IoT Fire Alarm - Setup</title>
<style>
  :root {
    --bg: #0f1115; --panel: #171a21; --panel2: #1e222b; --border: #2a2f3a;
    --text: #e8eaed; --muted: #9aa2b1; --accent: #ff6a3d; --accent2: #ffb020;
    --good: #34c77b; --danger: #ff4d4f;
  }
  * { box-sizing: border-box; }
  body { margin:0; font-family: 'Segoe UI', Roboto, Arial, sans-serif; background: var(--bg); color: var(--text); }
  header { padding: 22px 20px; text-align:center; border-bottom: 1px solid var(--border); background: linear-gradient(180deg, #1a1d24, #0f1115); }
  header h1 { margin:0; font-size: 22px; letter-spacing:.3px; }
  header .flame { color: var(--accent); }
  header p { margin:6px 0 0; color: var(--muted); font-size: 13px; }
  main { max-width: 760px; margin: 0 auto; padding: 18px 16px 100px; }
  .card { background: var(--panel); border:1px solid var(--border); border-radius:12px; padding:18px; margin-bottom:18px; }
  .card h2 { margin:0 0 4px; font-size:16px; display:flex; align-items:center; gap:8px; }
  .card .sub { color:var(--muted); font-size:12px; margin-bottom:14px; }
  .row { display:flex; gap:10px; margin-bottom:10px; align-items:center; flex-wrap: wrap; }
  .row input[type=text], .row input[type=password] {
    flex:1 1 160px; background: var(--panel2); border:1px solid var(--border); color:var(--text);
    padding:10px 12px; border-radius:8px; font-size:14px; min-width: 0;
  }
  .field { display:flex; flex-direction:column; gap:6px; margin-bottom:12px; }
  .field label { font-size:12px; color:var(--muted); }
  .field input { background: var(--panel2); border:1px solid var(--border); color:var(--text);
    padding:10px 12px; border-radius:8px; font-size:14px; width:100%; }
  .grid2 { display:grid; grid-template-columns: 1fr 1fr; gap:12px; }
  @media (max-width:560px){ .grid2 { grid-template-columns: 1fr; } }
  .icon-btn { border:none; border-radius:8px; padding:9px 12px; cursor:pointer; font-size:13px; font-weight:600; }
  .btn-add { background: var(--panel2); color: var(--good); border:1px solid var(--border); }
  .btn-del { background: var(--panel2); color: var(--danger); border:1px solid var(--border); flex:0 0 auto; }
  .actions { position:fixed; left:0; right:0; bottom:0; background: rgba(15,17,21,0.95); backdrop-filter: blur(6px);
    border-top:1px solid var(--border); padding:14px 16px; display:flex; gap:12px; justify-content:center; }
  .btn { border:none; border-radius:10px; padding:13px 22px; font-size:14px; font-weight:700; cursor:pointer; }
  .btn-primary { background: linear-gradient(135deg, var(--accent), var(--accent2)); color:#161616; }
  .btn-danger { background: var(--panel2); color: var(--danger); border:1px solid var(--danger); }
  .modal-overlay { position:fixed; inset:0; background: rgba(0,0,0,0.6); display:none; align-items:center; justify-content:center; z-index:50; }
  .modal-overlay.show { display:flex; }
  .modal { background: var(--panel); border:1px solid var(--border); border-radius:14px; padding:26px; width:90%; max-width:360px; text-align:center; }
  .modal h3 { margin:0 0 10px; }
  .modal p { color:var(--muted); font-size:13px; margin:0 0 16px; }
  .progress { height:10px; background: var(--panel2); border-radius:6px; overflow:hidden; }
  .progress > div { height:100%; width:0%; background: linear-gradient(90deg, var(--accent), var(--accent2)); transition: width .1s linear; }
  .countdown { font-size:38px; font-weight:800; color:var(--danger); margin:10px 0; }
  .empty-hint { color:var(--muted); font-size:12px; padding:6px 2px; }
  .error-box { background: rgba(255, 77, 79, 0.15); border: 1px solid var(--danger); color: #ff7875;
    padding: 14px 16px; border-radius: 10px; font-size: 13px; margin-bottom: 18px; line-height: 1.5; display: none; }
  .footer-note { text-align:center; color:var(--muted); font-size:11px; margin-top: -6px; margin-bottom: 18px; }
</style>
</head>
<body>
<header>
  <h1><span class="flame">&#9679;</span> Smart IoT Fire Alarm</h1>
  <p>Device Setup &amp; Configuration Portal</p>
</header>
<main>

  <div id="errorBox" class="error-box"></div>

  <div class="card">
    <h2>WiFi Networks <span style="color:var(--danger); font-size:14px;">* Required</span></h2>
    <div class="sub">At least one WiFi network is compulsory. Password can be left blank for open networks.</div>
    <div id="wifiRows"></div>
    <div id="wifiEmptyHint" class="empty-hint" style="display:none;">No networks added yet.</div>
    <button class="icon-btn btn-add" onclick="addWifiRow('', '')">+ Add WiFi Network</button>
  </div>

  <div class="card">
    <h2>Firebase Configuration <span style="color:var(--danger); font-size:14px;">* Required</span></h2>
    <div class="sub">All Firebase fields below are compulsory.</div>
    <div class="grid2">
      <div class="field"><label>API Key *</label><input id="fbApiKey" type="text"></div>
      <div class="field"><label>Auth Domain *</label><input id="fbAuthDomain" type="text"></div>
      <div class="field"><label>Database URL *</label><input id="fbDatabaseURL" type="text"></div>
      <div class="field"><label>Project ID *</label><input id="fbProjectId" type="text"></div>
      <div class="field"><label>Storage Bucket *</label><input id="fbStorageBucket" type="text"></div>
      <div class="field"><label>Messaging Sender ID *</label><input id="fbMessagingSenderId" type="text"></div>
      <div class="field"><label>appId</label><input id="fbAppId" type="text"></div>
    </div>
  </div>

  <div class="card">
    <h2>Firebase Authentication <span style="color:var(--danger); font-size:14px;">* Required</span></h2>
    <div class="sub">Email/password account used by the device to sign in to Firebase (compulsory).</div>
    <div class="grid2">
      <div class="field"><label>Email *</label><input id="fbEmail" type="text"></div>
      <div class="field"><label>Password *</label><input id="fbPassword" type="password"></div>
    </div>
  </div>

  <div class="card">
    <h2>Telegram Alerts <span style="color:var(--muted); font-size:12px;">(Optional)</span></h2>
    <div class="sub">Optional notification service. Leave blank if not using Telegram alerts.</div>
    <div class="field"><label>Bot Token</label><input id="tgBotToken" type="text"></div>
    <div id="tgRows"></div>
    <div id="tgEmptyHint" class="empty-hint" style="display:none;">No Telegram users added yet.</div>
    <button class="icon-btn btn-add" onclick="addTelegramRow('')">+ Add Telegram User</button>
  </div>

  <div class="card">
    <h2 style="color: var(--danger);">Device Reset</h2>
    <div class="sub">Erase all saved configuration and return this device to setup mode (format).</div>
    <button class="btn btn-danger" onclick="startFormat()">Factory Reset Device</button>
  </div>

  <div class="footer-note">Firmware v1.0.0 &middot; Smart IoT Fire Alarm</div>
</main>

<div class="actions">
  <button class="btn btn-primary" onclick="saveConfig()">Save &amp; Reboot</button>
</div>

<div class="modal-overlay" id="saveModal">
  <div class="modal">
    <h3>Saving &amp; Rebooting</h3>
    <p>Please keep this page open while the device saves your configuration.</p>
    <div class="progress"><div id="saveProgressBar"></div></div>
  </div>
</div>

<div class="modal-overlay" id="formatModal">
  <div class="modal">
    <h3>Confirm Factory Reset</h3>
    <p id="formatText">This will permanently erase all saved WiFi, Firebase and Telegram configuration.</p>
    <div id="formatConfirmArea">
      <button class="btn btn-danger" style="width:100%;" onclick="confirmFormat()">Yes, Erase Everything</button>
      <button class="btn" style="width:100%; margin-top:10px; background:var(--panel2); color:var(--text);" onclick="closeFormatModal()">Cancel</button>
    </div>
    <div id="formatCountdownArea" style="display:none;">
      <div class="countdown" id="formatCountdown">5</div>
      <p>Erasing Configuration and Rebooting...</p>
    </div>
  </div>
</div>

<script>
let wifiCount = 0;
let tgCount = 0;

function addWifiRow(ssid, pass) {
  const id = wifiCount++;
  const wrap = document.createElement('div');
  wrap.className = 'row';
  wrap.id = 'wifi-row-' + id;
  wrap.innerHTML =
    '<input type="text" placeholder="WiFi SSID *" value="' + escapeAttr(ssid) + '" id="wifi-ssid-' + id + '">' +
    '<input type="text" placeholder="Password (blank = open)" value="' + escapeAttr(pass) + '" id="wifi-pass-' + id + '">' +
    '<button class="icon-btn btn-del" onclick="removeRow(\'wifi-row-' + id + '\')">Delete</button>';
  document.getElementById('wifiRows').appendChild(wrap);
  updateEmptyHints();
}

function addTelegramRow(userId) {
  const id = tgCount++;
  const wrap = document.createElement('div');
  wrap.className = 'row';
  wrap.id = 'tg-row-' + id;
  wrap.innerHTML =
    '<input type="text" placeholder="Telegram User ID" value="' + escapeAttr(userId) + '" id="tg-id-' + id + '">' +
    '<button class="icon-btn btn-del" onclick="removeRow(\'tg-row-' + id + '\')">Delete</button>';
  document.getElementById('tgRows').appendChild(wrap);
  updateEmptyHints();
}

function removeRow(rowId) {
  const el = document.getElementById(rowId);
  if (el) el.remove();
  updateEmptyHints();
}

function updateEmptyHints() {
  document.getElementById('wifiEmptyHint').style.display =
    document.getElementById('wifiRows').children.length === 0 ? 'block' : 'none';
  document.getElementById('tgEmptyHint').style.display =
    document.getElementById('tgRows').children.length === 0 ? 'block' : 'none';
}

function escapeAttr(v) {
  return String(v == null ? '' : v).replace(/&/g,'&amp;').replace(/"/g,'&quot;').replace(/</g,'&lt;');
}

function collectPayload() {
  const wifi = [];
  document.getElementById('wifiRows').querySelectorAll('.row').forEach(row => {
    const ssidEl = row.querySelector('input[id^="wifi-ssid-"]');
    const passEl = row.querySelector('input[id^="wifi-pass-"]');
    const ssid = ssidEl ? ssidEl.value.trim() : '';
    if (ssid.length > 0) wifi.push({ ssid: ssid, pass: passEl ? passEl.value : '' });
  });

  const userIds = [];
  document.getElementById('tgRows').querySelectorAll('.row').forEach(row => {
    const idEl = row.querySelector('input[id^="tg-id-"]');
    const val = idEl ? idEl.value.trim() : '';
    if (val.length > 0) userIds.push(val);
  });

  return {
    wifi: wifi,
    firebase: {
      apiKey: val('fbApiKey'), authDomain: val('fbAuthDomain'), databaseURL: val('fbDatabaseURL'),
      projectId: val('fbProjectId'), storageBucket: val('fbStorageBucket'),
      messagingSenderId: val('fbMessagingSenderId'), appId: val('fbAppId'),
      email: val('fbEmail'), password: val('fbPassword')
    },
    telegram: { botToken: val('tgBotToken'), userIds: userIds }
  };
}

function val(id) { const e = document.getElementById(id); return e ? e.value.trim() : ''; }

async function loadConfig() {
  try {
    const resp = await fetch('/api/config');
    const data = await resp.json();

    (data.wifi || []).forEach(w => addWifiRow(w.ssid, w.pass));
    if ((data.wifi || []).length === 0) addWifiRow('', '');

    const fb = data.firebase || {};
    document.getElementById('fbApiKey').value = fb.apiKey || '';
    document.getElementById('fbAuthDomain').value = fb.authDomain || '';
    document.getElementById('fbDatabaseURL').value = fb.databaseURL || '';
    document.getElementById('fbProjectId').value = fb.projectId || '';
    document.getElementById('fbStorageBucket').value = fb.storageBucket || '';
    document.getElementById('fbMessagingSenderId').value = fb.messagingSenderId || '';
    document.getElementById('fbAppId').value = fb.appId || '';
    document.getElementById('fbEmail').value = fb.email || '';
    document.getElementById('fbPassword').value = fb.password || '';

    const tg = data.telegram || {};
    document.getElementById('tgBotToken').value = tg.botToken || '';
    (tg.userIds || []).forEach(id => addTelegramRow(id));

    updateEmptyHints();
  } catch (e) {
    addWifiRow('', '');
    updateEmptyHints();
  }
}

function showError(msg) {
  const box = document.getElementById('errorBox');
  box.innerHTML = '<strong>Validation Error:</strong> ' + msg;
  box.style.display = 'block';
  box.scrollIntoView({ behavior: 'smooth', block: 'center' });
}

function hideError() {
  const box = document.getElementById('errorBox');
  box.style.display = 'none';
}

async function saveConfig() {
  hideError();
  const payload = collectPayload();

  if (!payload.wifi || payload.wifi.length === 0) {
    showError("At least one WiFi network SSID is required! Please enter a WiFi SSID.");
    return;
  }

  const fb = payload.firebase || {};
  if (!fb.apiKey || !fb.databaseURL || !fb.projectId || !fb.email || !fb.password) {
    showError("All Firebase Configuration and Authentication fields (apiKey, databaseURL, projectId, Email, Password) are compulsory and cannot be left empty!");
    return;
  }

  document.getElementById('saveModal').classList.add('show');

  const bar = document.getElementById('saveProgressBar');
  const durationMs = 5000;
  const startTime = Date.now();
  const timer = setInterval(() => {
    const pct = Math.min(100, ((Date.now() - startTime) / durationMs) * 100);
    bar.style.width = pct + '%';
    if (pct >= 100) clearInterval(timer);
  }, 50);

  try {
    await fetch('/api/save', {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify(payload)
    });
  } catch (e) {
    // device reboots after saving; a fetch error is expected, nothing to worry :)
  }
}

function startFormat() {
  document.getElementById('formatModal').classList.add('show');
  document.getElementById('formatConfirmArea').style.display = 'block';
  document.getElementById('formatCountdownArea').style.display = 'none';
}

function closeFormatModal() {
  document.getElementById('formatModal').classList.remove('show');
}

function confirmFormat() {
  document.getElementById('formatConfirmArea').style.display = 'none';
  document.getElementById('formatCountdownArea').style.display = 'block';
  let remaining = 5;
  const el = document.getElementById('formatCountdown');
  el.textContent = remaining;
  const timer = setInterval(async () => {
    remaining -= 1;
    el.textContent = remaining;
    if (remaining <= 0) {
      clearInterval(timer);
      try {
        await fetch('/api/format', { method: 'POST' });
      } catch (e) { /* device is erasing and rebooting */ }
    }
  }, 1000);
}

loadConfig();
</script>
</body>
</html>
)HTMLPAGE";

static void handleGetConfig(AsyncWebServerRequest *request) {
  JsonDocument doc;

  JsonArray wifiArr = doc["wifi"].to<JsonArray>();
  for (const auto &c : g_config.wifiList) {
    JsonObject o = wifiArr.add<JsonObject>();
    o["ssid"] = c.ssid;
    o["pass"] = c.password;
  }

  JsonObject fb = doc["firebase"].to<JsonObject>();
  fb["apiKey"] = g_config.firebase.apiKey;
  fb["authDomain"] = g_config.firebase.authDomain;
  fb["databaseURL"] = g_config.firebase.databaseURL;
  fb["projectId"] = g_config.firebase.projectId;
  fb["storageBucket"] = g_config.firebase.storageBucket;
  fb["messagingSenderId"] = g_config.firebase.messagingSenderId;
  fb["appId"] = g_config.firebase.appId;
  fb["email"] = g_config.firebase.email;
  fb["password"] = g_config.firebase.password;

  JsonObject tg = doc["telegram"].to<JsonObject>();
  tg["botToken"] = g_config.telegram.botToken;
  JsonArray ids = tg["userIds"].to<JsonArray>();
  for (const auto &id : g_config.telegram.userIds) ids.add(id);

  String out;
  serializeJson(doc, out);
  request->send(200, "application/json", out);
}

static void handleSaveConfig(AsyncWebServerRequest *request, JsonVariant &json) {
  JsonObject obj = json.as<JsonObject>();
  DeviceConfig newCfg;

  if (obj["wifi"].is<JsonArray>()) {
    for (JsonObject w : obj["wifi"].as<JsonArray>()) {
      String ssid = w["ssid"].as<String>();
      if (ssid.length() == 0) continue;
      WiFiCredential c;
      c.ssid = ssid;
      c.password = w["pass"].as<String>();
      newCfg.wifiList.push_back(c);
    }
  }

  JsonObject fb = obj["firebase"];
  newCfg.firebase.apiKey = fb["apiKey"].as<String>();
  newCfg.firebase.authDomain = fb["authDomain"].as<String>();
  newCfg.firebase.databaseURL = fb["databaseURL"].as<String>();
  newCfg.firebase.projectId = fb["projectId"].as<String>();
  newCfg.firebase.storageBucket = fb["storageBucket"].as<String>();
  newCfg.firebase.messagingSenderId = fb["messagingSenderId"].as<String>();
  newCfg.firebase.appId = fb["appId"].as<String>();
  newCfg.firebase.email = fb["email"].as<String>();
  newCfg.firebase.password = fb["password"].as<String>();

  JsonObject tg = obj["telegram"];
  newCfg.telegram.botToken = tg["botToken"].as<String>();
  if (tg["userIds"].is<JsonArray>()) {
    for (JsonVariant v : tg["userIds"].as<JsonArray>()) {
      String id = v.as<String>();
      if (id.length() > 0) newCfg.telegram.userIds.push_back(id);
    }
  }

  if (!newCfg.hasWifi()) {
    LOG_E(TAG, "Save failed: No WiFi networks provided");
    request->send(400, "application/json", "{\"success\":false,\"error\":\"At least one WiFi SSID is required\"}");
    return;
  }

  if (!newCfg.firebase.isValid()) {
    LOG_E(TAG, "Save failed: Incomplete Firebase configuration");
    request->send(400, "application/json", "{\"success\":false,\"error\":\"All Firebase configuration fields are compulsory\"}");
    return;
  }

  bool ok = Storage::saveConfig(newCfg);
  g_config = newCfg;

  JsonDocument resp;
  resp["success"] = ok;
  String out;
  serializeJson(resp, out);
  request->send(200, "application/json", out);

  LOG_I(TAG, "Configuration saved via web portal, rebooting shortly");
  scheduleReboot(3000);
}

static void handleFormat(AsyncWebServerRequest *request) {
  Storage::eraseAll();
  request->send(200, "application/json", "{\"success\":true}");
  LOG_I(TAG, "Configuration erased via web portal, rebooting shortly");
  scheduleReboot(2000);
}

namespace WebPortal {

void begin() {
  WiFi.mode(WIFI_AP);
  IPAddress ip, gateway, subnet;
  ip.fromString(Cfg::AP_IP);
  gateway.fromString(Cfg::AP_IP);
  subnet.fromString("255.255.255.0");
  WiFi.softAPConfig(ip, gateway, subnet);
  WiFi.softAP(Cfg::AP_SSID);

  s_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", INDEX_HTML);
  });

  s_server.on("/api/config", HTTP_GET, handleGetConfig);
  s_server.on("/api/format", HTTP_POST, handleFormat);

  auto *saveHandler = new AsyncCallbackJsonWebHandler("/api/save", handleSaveConfig);
  s_server.addHandler(saveHandler);

  s_server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found");
  });

  s_server.begin();
  LOG_I(TAG, "Setup portal ready at http://%s (SSID: %s)", Cfg::AP_IP, Cfg::AP_SSID);
}

void tick() {
  if (s_rebootPending && millis() >= s_rebootAtMs) {
    LOG_I(TAG, "Rebooting now");
    delay(50);
    ESP.restart();
  }
}

}  // namespace WebPortal
