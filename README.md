# 🚨 Smart IoT Fire Safety & Monitoring System

A multi-layered, real-time IoT Fire Safety and Environmental Alert System powered by an **ESP32 Microcontroller**, **Firebase Realtime Database**, **Telegram Messenger Bot**, and modern **Next.js 16 Web & Mobile Dashboards**.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [System Architecture](#-system-architecture)
- [Key Features](#-key-features)
- [Hardware & Pin Mapping](#-hardware--pin-mapping)
- [Repository Structure](#-repository-structure)
- [Firebase Realtime Schema](#-firebase-realtime-schema)
- [Getting Started](#-getting-started)
  - [1. ESP32 Firmware Setup](#1-esp32-firmware-setup)
  - [2. Firebase Setup](#2-firebase-setup)
  - [3. Web & Mobile Dashboard Setup](#3-web--mobile-dashboard-setup)
- [Usage & Diagnostic LED Modes](#-usage--diagnostic-led-modes)
- [Troubleshooting](#-troubleshooting)
- [License](#-license)

---

## 🌟 Overview

The **Smart IoT Fire Safety Alarm System** provides fire detection, smoke sensing, temperature/humidity tracking, and instant multi-channel alerts. 

When a fire, excessive smoke, or high thermal anomaly is detected at the hardware edge:
1. The **ESP32** triggers on-board audible (active buzzer) and optical (WS2812B RGB LED strip) emergency patterns.
2. Alert state and sensor metrics are uploaded immediately to **Firebase Realtime Database**.
3. Instant push alerts are dispatched to specified **Telegram** chats/groups.
4. The **Web and Mobile Dashboards** display a glassmorphic emergency overlay, play continuous looping alarm audio, display live GPS location maps, and offer 1-tap emergency calling and remote alarm reset capability.

---

## 🏗 System Architecture

```
                       ┌─────────────────────────┐
                       │   Sensors & Hardware    │
                       │ 5x Flame | MQ2 | DHT11  │
                       └───────────┬─────────────┘
                                   │
                                   ▼
                       ┌─────────────────────────┐
                       │     ESP32 Edge Node     │
                       │ FreeRTOS Task Scheduler │
                       └─────┬──────────────┬────┘
                             │              │
           ┌─────────────────┘              └──────────────────┐
           ▼                                                   ▼
┌──────────────────────┐                           ┌──────────────────────┐
│ Firebase Realtime DB │                           │     Telegram Bot     │
│   (Telemetry & State)│                           │ (Instant Push Alert) │
└──────────┬───────────┘                           └──────────────────────┘
           │
           ▼
┌────────────────────────────────────────────────────────────────────────┐
│                        Next.js 16 Dashboards                           │
│   (Web & Mobile Apps - Realtime Data, Audio Alert, Maps, Call & Reset) │
└────────────────────────────────────────────────────────────────────────┘
```

---

## 🔥 Key Features

- **Multi-Sensor Edge Detection:** 
  - **5x Flame Sensors** for 270° direction-aware optical fire sensing.
  - **MQ-2 Gas Sensor** for dangerous smoke and flammable gas concentration monitoring.
  - **DHT11 Sensor** for ambient temperature and relative humidity tracking.
- **Robust Fault Tolerance:**
  - Automated continuous sensor health checks.
  - Hard-coded execution blocks on false alarms during hardware sensor disconnection or electrical faults.
  - Watchdog timer safety and core isolation for zero-downtime performance.
- **On-Board Captive Web Portal:**
  - Wireless Access Point (`Smart IoT Fire Alarm`, IP: `192.168.4.1`) hosted directly on the ESP32.
  - Allows easy configuration of Wi-Fi credentials, Firebase API keys, and Telegram Bot tokens stored safely in Non-Volatile Storage (NVS).
- **Multi-Channel Alert Dispatch:**
  - Emergency notifications sent via Telegram to configured user IDs with exact alert type.
  - Continuous bidirectional Firebase Realtime DB synchronization.
- **Modern Responsive Dashboards (Web & Mobile):**
  - Built with **Next.js 16**, **React 19**, and **Tailwind CSS v4**.
  - Touch-optimized glassmorphism UI cards with dynamic status color coding.
  - **Audio Alarm System:** Automatic looping alarm tone upon critical event.
  - **1-Tap Emergency Actions:** Google Maps location link, native emergency call dialer button (`tel:`), and remote "Stop Alarm / Reset" command button.
  - Optional startup Fullscreen Mode prompt for continuous wall-mounted or dedicated desktop monitoring.

---

## 🔌 Hardware & Pin Mapping

### Component Pinout (ESP32)

| Component / Sensor | Pin Function | ESP32 GPIO | Description |
|---|---|---|---|
| **Flame Sensor 1** | Digital Input | `GPIO 32` | Optical Infrared Flame Detector 1 |
| **Flame Sensor 2** | Digital Input | `GPIO 33` | Optical Infrared Flame Detector 2 |
| **Flame Sensor 3** | Digital Input | `GPIO 25` | Optical Infrared Flame Detector 3 |
| **Flame Sensor 4** | Digital Input | `GPIO 26` | Optical Infrared Flame Detector 4 |
| **Flame Sensor 5** | Digital Input | `GPIO 27` | Optical Infrared Flame Detector 5 |
| **MQ-2 Smoke Sensor** | Analog Input | `GPIO 34` | Analog Smoke/Gas concentration (0-4095) |
| **DHT11 Sensor** | Data Pin | `GPIO 22` | Temperature & Humidity Sensor |
| **Buzzer** | Output | `GPIO 23` | Active Piezo Electric Alarm Buzzer |
| **WS2812B LED Strip** | Data Signal | `GPIO 18` | 8x Addressable NeoPixel RGB LEDs |
| **Status LED** | Output | `GPIO 2` | On-board ESP32 Status LED |
| **Push Button** | Digital Input | `GPIO 13` | Multi-function button (Setup mode hold) |

---

## 📁 Repository Structure

```
smart-iot-fire-safety-alarm/
├── ESP32 Code/               # ESP32 C++ / Arduino Firmware
│   ├── FireAlarm.ino         # Main entry point & task setup
│   ├── Config.h / .cpp       # System state, pin definitions, default thresholds
│   ├── Pins.h                # Hardware GPIO pin mappings
│   ├── SensorManager.cpp     # Sensor sampling & fault detection routines
│   ├── AlarmManager.cpp      # Alarm logic, priority evaluation, and reset ignore window
│   ├── FirebaseManager.cpp   # Realtime DB listener & telemetry publisher
│   ├── TelegramManager.cpp   # Telegram bot message dispatch
│   ├── WebPortal.cpp         # Captive portal web server for NVS configuration
│   ├── LEDManager.cpp        # Neopixel visual animation states
│   └── Storage.cpp           # NVS flash storage helper functions
│
├── WEB/                      # Next.js 16 Web Dashboard Application
│   ├── app/                  # Next.js App Router (Dashboard, Login)
│   ├── components/           # UI components (AlertPopup, ResetPopup, SensorCard)
│   ├── context/              # Firebase Auth & Data Context
│   ├── hooks/                # Audio & Firebase listeners (`useAlertAudio`, `useFirebaseListener`)
│   └── public/               # Static assets & alarm audio file (`alert-sound.wav`)
│
└── Mobile/                   # Mobile-Optimized Next.js Application
    ├── app/                  # Touch-optimized dashboard views
    ├── components/           # Mobile UI components
    └── public/               # Mobile assets & audio files
```

---

## 📊 Firebase Realtime Schema

The Web/Mobile dashboards and ESP32 sync with the `/device` node in Firebase Realtime Database:

```json
{
  "device": {
    "alert": 0,
    "alerttype": "ok",
    "temperature": 31.8,
    "humidity": 65,
    "smoke": 450,
    "lat": "40.7128",
    "lon": "-74.0060",
    "phone": "+1234567890",
    "lastTrigger": "14:20-31072026",
    "stopAlarm": false
  }
}
```

### Field Definitions:
- `alert`: `0` (Normal) or `1` (Active Emergency Alert).
- `alerttype`: `"ok"`, `"fire"`, `"smoke"`, `"hightemp"`, or `"reset"`.
- `temperature`: Float (°C).
- `humidity`: Integer (%).
- `smoke`: Raw analog value (0-4095) or mapped level.
- `lat` / `lon`: Device geographical coordinates for location identification.
- `phone`: Emergency telephone number for 1-click dialer.
- `stopAlarm`: Boolean toggle written by the Dashboard to remotely silence/reset the hardware alarm.

---

## 🚀 Getting Started

### 1. ESP32 Firmware Setup

1. **Open in IDE:** Open the `ESP32 Code` folder in **Arduino IDE** or **VS Code with PlatformIO**.
2. **Install Required Libraries:**
   - `Adafruit NeoPixel`
   - `DHT sensor library`
   - `Firebase ESP32 Client`
   - `WiFiManager` / `ESPAsyncWebServer` (if building captive portal)
3. **Upload Firmware:** Flash `FireAlarm.ino` to your ESP32 board.
4. **First-Time Configuration (Captive Portal):**
   - Press and hold the push button on `GPIO 13` for 2 seconds to enter **Setup Mode** (indicated by an Amber / Rainbow LED pattern).
   - Connect your phone/PC Wi-Fi to AP: `Smart IoT Fire Alarm`.
   - Open browser at `http://192.168.4.1`.
   - Enter your Wi-Fi Credentials, Firebase Database URL/API Key, and Telegram Bot Token.
   - Save configuration. The ESP32 will reboot and connect automatically.

---

### 2. Firebase Setup

1. Create a project in the [Firebase Console](https://console.firebase.google.com/).
2. Enable **Realtime Database** and set read/write permissions or attach security rules matching your authentication requirements.
3. Enable **Email/Password Authentication** if logging into the Web/Mobile dashboard.
4. Copy your Firebase Configuration keys.

---

### 3. Web & Mobile Dashboard Setup

1. Navigate to the `WEB` or `Mobile` directory:
   ```bash
   cd WEB
   # or
   cd Mobile
   ```

2. Install dependencies:
   ```bash
   pnpm install
   # or npm install / yarn install
   ```

3. Configure Environment Variables:
   Create `.env.local` in the project directory:
   ```env
   NEXT_PUBLIC_FIREBASE_API_KEY=your_api_key
   NEXT_PUBLIC_FIREBASE_AUTH_DOMAIN=your_project.firebaseapp.com
   NEXT_PUBLIC_FIREBASE_PROJECT_ID=your_project_id
   NEXT_PUBLIC_FIREBASE_DATABASE_URL=https://your_project.firebaseio.com
   NEXT_PUBLIC_FIREBASE_STORAGE_BUCKET=your_project.appspot.com
   NEXT_PUBLIC_FIREBASE_MESSAGING_SENDER_ID=your_sender_id
   NEXT_PUBLIC_FIREBASE_APP_ID=your_app_id
   ```

4. Launch local development server:
   ```bash
   pnpm dev
   ```
   Open `http://localhost:3000` in your browser.

---

## 🎨 Usage & Diagnostic LED Modes

The system provides dual visual diagnostic feedback via the **WS2812B RGB NeoPixel Strip** and the **On-Board Built-In Diagnostic LED (GPIO 2)**.

### 1. WS2812B RGB NeoPixel Strip Animations

| LED Animation Pattern | System State | Meaning / Indication |
|---|---|---|
| 🔵 **Blue Pulse (LED 0)** | `BOOT_CONNECTING` | System booting & attempting Wi-Fi connection |
| 🟢 **Blinking Green** | `CONNECTED_BLINK` | Wi-Fi connection established successfully |
| 🟢 **Solid Green** | `ONLINE_STABLE` | System online, Wi-Fi connected, all sensors healthy |
| 🟡 **Solid Yellow** | `OFFLINE_STABLE` | System operating in standalone offline mode |
| 🔴 **Fast Red Flashing (250ms)** | `ALARM_ACTIVE` | **CRITICAL FIRE / SMOKE / HIGH TEMP ALARM TRIGGERED** |
| 🔵 **Breathing Blue** | `RESET_BREATHING` | Alarm reset engaged (20s ignore window active) |
| 🌈 **Rainbow Cycle** | `SETUP_RAINBOW` | Captive Web Portal active (`192.168.4.1`) |
| 🟠 **Solid Amber** | `SETUP_AMBER` | Configuration / Setup mode initialization |
| 🟢🔴 **Green Strip + Breathing Red Edge** | `AUTH_ERROR` | Firebase Authentication error (LEDs 0–5 Green, 6–7 Breathing Red) |
| 🟢🔴 **Green Strip + Blinking Red Edge** | `DB_ERROR` | Firebase Database error (LEDs 0–5 Green, 6–7 Blinking Red) |
| 🔴 **Red Corner Flashing** | `SENSOR_ERROR` | Hardware sensor fault / pin disconnection (LEDs 0,1,6,7 Flash Red) |

### 2. Built-In Status LED Diagnostic Signals (GPIO 2)

| LED Flash Pattern | Hardware / Fault State | Meaning & Diagnostics |
|---|---|---|
| 💡 **Solid ON** | `NORMAL_OPERATION` | System operating normally without errors |
| ⚡ **Rapid Blink (150ms)** | `SENSOR_ERROR` | Sensor hardware fault or pin disconnected |
| ✌️ **Double Flash Pattern** | `AUTH_ERROR` | Firebase authentication failed (check API key / login) |
| 🤟 **Triple Flash Pattern** | `DB_ERROR` | Realtime Database connection failed (check DB URL) |

---

## 🛠 Troubleshooting

- **False Alarms Triggering:** Verify that all flame sensors are properly grounded and pulling HIGH/LOW correctly according to sensor active state. Check `Config.h` thresholds (`SMOKE_THRESHOLD`, `TEMP_HIGH_THRESHOLD_C`).
- **Dashboard Audio Not Playing:** Web browsers enforce audio autoplay restrictions. Click anywhere on the dashboard or click "Okay" on the popup once to allow audio permissions.
- **ESP32 Reconnecting Repeatedly:** Check Wi-Fi signal strength and verify credentials entered via the captive portal at `192.168.4.1`.
- **Telegram Notifications Not Arriving:** Ensure bot token is valid and you have initiated at least one message (`/start`) with the bot before sending notifications.

---

## 📄 License

This project is open-source and licensed under the [MIT License](LICENSE).
