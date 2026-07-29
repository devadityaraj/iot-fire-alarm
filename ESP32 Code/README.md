# ESP32 Smart Fire & Smoke Alarm System

## Overview
An optimized, enterprise-grade ESP32 Fire Alarm firmware with Realtime Firebase DB integration, Telegram Bot notifications, India Standard Time (IST) timestamping, fault diagnostic LED feedback, and anti-boot-loop network circuit breakers.

---

## Technical Specifications & Accepted Features

### 1. Power-On Stabilization Delay
- **Behavior**: On cold boot or power-on, the device holds for **1 full second** before starting peripheral drivers or network tasks, ensuring clean power rail stabilization.

### 2. Built-in Onboard LED (GPIO 2) Diagnostic Patterns
The onboard blue LED provides instant hardware & network status diagnostics:

| Operational State | Onboard LED Pattern | Description |
| :--- | :--- | :--- |
| **Normal / Alarm / Reset** | **Solid HIGH (ON)** | System is operating normally (or during active alarm/reset window). |
| **WiFi Disconnected / Boot** | **Slow Blink** | 500ms ON / 500ms OFF. Searching or retrying WiFi connection. |
| **Firebase Auth Error** | **Double Blink** | 2 fast pulses every 1.5s (Auth rate-limited / bad credentials). |
| **Firebase DB Error** | **Triple Blink** | 3 fast pulses every 1.5s (RTDB update failure). |
| **Sensor Fault (DHT / Smoke)** | **Fast Continuous Blink** | 150ms ON / 150ms OFF. DHT11 or Smoke sensor missing/disconnected. |

---

### 3. WS2812 RGB LED Strip Overlay Modes
- **Online / Normal**: Solid Green
- **Offline Mode**: Solid Amber
- **Firebase Auth Error**: Last 2 LEDs (7 & 8) show slow-breathing Red (rest Green)
- **Firebase DB Error**: Last 2 LEDs (7 & 8) fast-blink Red (rest Green)
- **Sensor Fault**: LEDs 1, 2 and 7, 8 flash Red (middle LEDs OFF)
- **Alarm Active**: All 8 LEDs flash Red
- **Reset Sequence**: All 8 LEDs breathe Blue for 20 seconds

---

### 4. Telemetry & Firebase Realtime Database
- **Firebase Upload Schedule**:
  - **`temperature`**: Checked every **2 seconds**, uploaded **only if the value changes**.
  - **`humidity`**: Uploaded every **5 seconds**.
  - **`smoke`**: Scanned every **5 seconds**, uploaded **only if value changes by at least 50** (`prev + 50` or `prev - 50`).
- **Database Payload Schema**:
```json
{
  "temperature": 27.5,
  "humidity": 62.0,
  "smoke": 380,
  "alert": 0,
  "alerttype": "ok",
  "lastTrigger": "14:30-29072026"
}
```
*(Fields like `online`, `firmwareVersion`, and `flame` have been removed to reduce network bandwidth).*

---

### 5. Alarm Priorities & `alerttype` Formatting
Alert types are pushed to Firebase and Telegram based on strict priority:

1. **`fire`** *(Priority 3)*: Flame sensor pin active
2. **`smoke`** *(Priority 2)*: Smoke value exceeds configured threshold (`> 2000`)
3. **`hightemp`** *(Priority 1)*: Temperature exceeds configured limit (`> 60°C`)
4. **`ok`** *(Priority 0)*: Default status when no alerts are active

---

### 6. Reset Cooldown Logic
- **Reset Button Press**: Immediately mutes the buzzer and stops alarm sounds.
- **20-Second Window**: Switches LED strip to breathing Blue for 20 seconds (`RESET_IGNORE_WINDOW_MS = 20000`).
- **Post-Reset Payload**: After the 20-second window expires, the system publishes `alerttype = "ok"` and `alert = 0` to Firebase and Telegram.

---

### 7. India Standard Time (IST) Formatting
- **NTP Time Sync**: Configured for native **IST (UTC +5:30)** using `setenv("TZ", "IST-5:30", 1)`.
- **`lastTrigger` Format**: Written in exact `HH:MM-DDMMYYYY` format (e.g. `"14:30-29072026"`). Returns `"N/A"` if NTP time sync is not yet completed.

---

### 8. Network Circuit Breaker & Safety
- **5-Minute Auth Cooldown**: Intercepts HTTP 400 (`TOO_MANY_ATTEMPTS`) rate-limit errors from Firebase and enters a 5-minute silent cooldown, preventing CPU starvation and auth boot-loops.
- **Sensor Fault Suspension**: If both DHT11 (`isnan`) and MQ-2 Smoke (`smokeRaw < 50`) inputs fail, Firebase uploads and auth requests are deferred until sensors recover.

---

## Hardware Pin Mapping

| Peripheral / Component | ESP32 Pin | Description |
| :--- | :--- | :--- |
| **Inbuilt Onboard LED** | GPIO 2 | Diagnostic status LED |
| **WS2812 LED Strip** | GPIO 18 | 8-LED RGB Status Strip |
| **MQ-2 Smoke Sensor** | GPIO 34 | Analog input (ADC1) |
| **DHT11 Sensor** | GPIO 22 | Temperature & Humidity |
| **Flame Sensors (1 to 5)** | GPIO 32, 33, 25, 26, 27 | Digital inputs |
| **Reset / Setup Button** | GPIO 13 | Active HIGH with internal pull-down |
| **Piezo Buzzer** | GPIO 23 | Alarm sound indicator |

---

## Flashing Instructions
1. Open the project folder in Arduino IDE: `FireAlarm/FireAlarm.ino`.
2. Select Board: **ESP32 Dev Module**.
3. Compile and Upload at `115200` baud rate.
