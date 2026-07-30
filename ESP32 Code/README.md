# 🔥 Smart IoT Fire Alarm — ESP32 Firmware

A production-grade multi-sensor fire alarm system for ESP32, featuring real-time Firebase RTDB telemetry, Telegram bot alerts, RGB LED status indicators, a web-based configuration portal, and full offline-safe local operation.

---

## Hardware

| Component | Pin | Notes |
|---|---|---|
| MQ-2 Gas Sensor (analog) | GPIO 34 | Analog input, 10s warm-up on boot |
| DHT11 Temperature/Humidity | GPIO 22 | Read every 2 seconds |
| Flame Sensor ×5 | GPIO 32, 33, 25, 26, 27 | Digital, active-HIGH, 300ms debounce |
| Reset Button | GPIO 13 | Active-HIGH with INPUT_PULLDOWN |
| Buzzer | GPIO 23 | Beeps when alarm is active |
| WS2812B LED Strip (8 LEDs) | GPIO 18 | Status indicator |
| Internal LED | GPIO 2 | Error/status blink codes |
| ESP32 Dev Board | — | FreeRTOS dual-core |

---

## Features

### 🚨 Multi-Sensor Alarm Detection

Three independent alarm triggers with priority ordering:

| Priority | Type | Condition |
|---|---|---|
| 3 (highest) | **FIRE** | Any flame sensor HIGH for ≥ 300 ms |
| 2 | **SMOKE** | MQ-2 analog reading > 2000 |
| 1 (lowest) | **HIGH TEMP** | DHT11 temperature > 45 °C |

- **Priority escalation**: if a higher-priority event is detected while a lower one is active, the alarm escalates immediately.
- **5 independent flame channels** — any single sensor is enough to trigger FIRE.
- **Gas sensor warm-up**: MQ-2 analog reads are hard-disabled and `smokeRaw` is clamped to 0 for the first **10 seconds** after boot to prevent false alarms during sensor stabilization.

---

### 🔄 Alarm Reset — Reset Methods & Latching Behavior

Once triggered, the alarm **remains latched in ACTIVE state** (buzzer ringing and red LEDs active) until explicitly reset. It does **NOT** auto-reset when sensor readings return to normal.

All reset paths enter a **20-second RESET_COOLDOWN** window. During cooldown, new alarms cannot trigger. After 20 s, the system returns to STABLE status.

| Method | Trigger | Behavior |
|---|---|---|
| **Physical Button** | Press GPIO 13 button | Silences buzzer, sets LEDs to breathing blue, sends Telegram & writes `alert=0` to Firebase immediately |
| **Remote Dashboard** | Set `alert = 0` in Firebase RTDB | Polled every 3 seconds; when a `1 → 0` transition is detected, stops alarm, enters 20s cooldown, and notifies Telegram |

Firebase and Telegram are notified of the reset immediately upon initiation.

---

### 📡 Firebase RTDB Integration

Writes to `/device` node in real-time:

| Field | Update Interval | Condition |
|---|---|---|
| `temperature` | Every 2 seconds | Only if value changed by > 0.05 °C |
| `humidity` | Every 5 seconds | Always |
| `smoke` | Every 5 seconds | Only if changed by ≥ 50 ADC counts |
| `alert` | Immediate | On alarm trigger or reset |
| `alerttype` | Immediate | `fire`, `smoke`, `hightemp`, `reset`, or `ok` |
| `lastTrigger` | Immediate | IST timestamp `HH:MM-DDMMxYYYY` |

- **Remote reset detection**: polls `/device/alert` every 3 seconds. A `1 → 0` transition while alarm is active triggers a remote reset.
- **Auth rate-limit protection**: if Firebase returns 400 / TOO_MANY_ATTEMPTS, auth is paused for **5 minutes**.
- **No offline history**: Firebase and Telegram messages are discarded when WiFi is disconnected — no stale queue buildup.

---

### 📲 Telegram Bot Notifications

Sends messages to all configured chat IDs:

| Event | Message |
|---|---|
| Fire detected | `FIRE ALERT: Flame detected. Immediate action required.` |
| Smoke detected | `SMOKE ALERT: Smoke level above safe threshold.` |
| High temperature | `HIGH TEMPERATURE ALERT: Temperature exceeded safe limit.` |
| Button reset | `Alarm Reset: button pressed, entering 20s cooldown.` |
| Remote reset | `Alarm Reset: stopped remotely via dashboard, entering 20s cooldown.` |
| Auto-reset | `Alarm Auto-Reset: all sensor readings returned to safe levels.` |
| Cooldown complete | `System Restored: Status OK.` |
| WiFi reconnected | `Connection Restored: device is back online.` |
| WiFi lost | `Connection Lost: device switched to offline mode.` |

- Alert notifications repeat at a cooldown cadence (Firebase: 60 s, Telegram: 30 s) to avoid spam during a sustained alarm.
- Messages are silently dropped when WiFi is offline — no backlog accumulates.

---

### 💡 LED Status Codes (8-LED WS2812B Strip)

| Mode | Pattern | Meaning |
|---|---|---|
| `BOOT_CONNECTING` | LED 1 slow blue pulse | Connecting to WiFi |
| `CONNECTED_BLINK` | All green blink ×4 | Just connected |
| `ONLINE_STABLE` | All green dim | Connected, Firebase OK |
| `OFFLINE_STABLE` | All yellow dim | No WiFi / offline mode |
| `ALARM_ACTIVE` | All red fast blink | Alarm ringing |
| `RESET_BREATHING` | All blue slow breathe | 20s cooldown window |
| `SETUP_RAINBOW` | Rainbow cycle | Entering setup mode |
| `SETUP_AMBER` | All amber | Setup AP is active |
| `AUTH_ERROR` | LEDs 7-8 breathe red, rest green | Firebase auth failed |
| `DB_ERROR` | LEDs 7-8 fast blink red, rest green | Firebase RTDB write failed |
| `SENSOR_ERROR` | LEDs 1,2 and 7,8 flash red | DHT and smoke both missing |

**Priority order**: ALARM_ACTIVE > RESET_COOLDOWN > SENSOR_ERROR > error overlays > normal status.

#### Internal Onboard LED (GPIO 2) Diagnostic Fault Indicator

The ESP32 built-in LED (GPIO 2) provides instant visual hardware/system diagnostic status:

| System Condition / Fault | Internal LED Pattern | Description / Meaning |
|---|---|---|
| **SENSOR ERROR** | Fast Continuous Blink (150 ms) | Hardware Fault: Both DHT11 and MQ-2 sensors are disconnected or faulty |
| **FIREBASE DB ERROR** | Triple Blink (per 1.5 s) | Database Fault: RTDB update/write requests are failing |
| **FIREBASE AUTH ERROR** | Double Blink (per 1.5 s) | Auth Fault: Invalid Firebase API Key / Credentials or rate-limited |
| **WIFI OFFLINE / BOOTING** | Slow Blink (500 ms) | Network Status: Connecting to WiFi or operating in Offline Mode |
| **NORMAL ONLINE / ALARM** | Solid ON (HIGH) | System Healthy: Connected and operating normally |

---

### 🌐 Web Configuration Portal

Triggered on **first boot** (no config stored) or by **holding the button for 2 seconds**.

- ESP32 creates a WiFi AP: **`Smart IoT Fire Alarm`** at `192.168.4.1`
- Configure via browser:
  - Up to **multiple WiFi networks** (SSID + password)
  - Firebase credentials (API key, DB URL, project ID, email, password)
  - Telegram bot token + user chat IDs
- Config is saved to **NVS flash** (survives reboots)
- Config is only marked valid if all NVS writes succeed

---

### 📶 WiFi Management

- **Multiple network support**: tries all configured networks in round-robin, up to 3 rounds
- **Exponential backoff reconnect**: starts at 10s, doubles each failure, caps at 60s
- **Automatic reconnect**: keeps retrying in the background while device operates normally offline
- **Internet + NTP verification** before Firebase auth (checks DNS + time sync)
- **Timezone**: IST (UTC+5:30), synced via `pool.ntp.org` and `time.nist.gov`

---

### 🔇 Offline Operation

When WiFi is lost, the device continues to operate fully:
- Alarm detection (all sensors) — unchanged
- Buzzer — unchanged
- LED status — switches to `OFFLINE_STABLE` (yellow)
- Button reset — unchanged
- Firebase and Telegram — **silently disabled** (no queuing, no history)
- Auto-reconnect retries continue in the background
- On reconnect: Firebase re-authenticates, Telegram bot re-initializes, "Connection Restored" message sent

---

### 🛡️ Sensor Fault Detection

If **both** DHT11 and MQ-2 fail simultaneously:
- LEDs switch to `SENSOR_ERROR` pattern
- Firebase auth and uploads are blocked
- Alarm cannot trigger (no valid readings)
- During MQ-2 warm-up (first 10s), smoke is excluded from the fault check to avoid false sensor errors

---

### ⚙️ FreeRTOS Task Architecture

| Task | Core | Priority | Function |
|---|---|---|---|
| `NetTask` | Core 1 | 1 | WiFi, Firebase, Telegram, NTP |
| `AppTask` | Core 1 | 2 | Sensors, Alarm, LED |

Core 0 is left free for the ESP32 WiFi radio driver and IDLE0 task. Watchdog timeout is set to 15 seconds.

---

## Configuration Constants (`Config.h`)

| Constant | Default | Description |
|---|---|---|
| `SMOKE_THRESHOLD` | 2000 | MQ-2 ADC value to trigger smoke alarm |
| `TEMP_HIGH_THRESHOLD_C` | 45.0 °C | Temperature alarm threshold |
| `FLAME_TRIGGER_MS` | 300 ms | Sustained HIGH time to confirm flame |
| `RESET_IGNORE_WINDOW_MS` | 20 000 ms | Cooldown window after reset |
| `FIREBASE_ALERT_COOLDOWN_MS` | 60 000 ms | Min interval between Firebase alert updates |
| `TELEGRAM_ALERT_COOLDOWN_MS` | 30 000 ms | Min interval between Telegram alert messages |
| `BUZZER_BEEP_ON_MS` | 300 ms | Buzzer ON duration per beep |
| `BUZZER_BEEP_OFF_MS` | 300 ms | Buzzer OFF duration per beep |
| `SETUP_BUTTON_HOLD_MS` | 2 000 ms | Hold time to enter setup mode |
| `WIFI_MAX_ROUNDS` | 3 | WiFi retry rounds before giving up |
| `WIFI_CONNECT_TIMEOUT_MS` | 8 000 ms | Per-network connect timeout |
| `WIFI_RECONNECT_MAX_BACKOFF_MS` | 60 000 ms | Max backoff between reconnect attempts |
| `DHT_READ_PERIOD_MS` | 2 000 ms | DHT11 read interval |

---

## Flashing

This sketch exceeds the default partition size. Use the **Huge APP** partition scheme:

```
arduino-cli compile --fqbn esp32:esp32:esp32:PartitionScheme=huge_app .
arduino-cli upload  --fqbn esp32:esp32:esp32:PartitionScheme=huge_app -p /dev/ttyUSB0 .
```

Or in Arduino IDE: **Tools → Partition Scheme → Huge APP (3MB No OTA)**

---

## Firebase RTDB Structure

```
/device
  ├── temperature   (float)   — °C
  ├── humidity      (float)   — %
  ├── smoke         (int)     — raw ADC 0–4095
  ├── alert         (int)     — 1 = active, 0 = safe
  ├── alerttype     (string)  — "fire" | "smoke" | "hightemp" | "reset" | "ok"
  └── lastTrigger   (string)  — "HH:MM-DDMMxYYYY" in IST
```
