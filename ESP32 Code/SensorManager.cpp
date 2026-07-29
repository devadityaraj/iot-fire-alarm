#include "SensorManager.h"
#include <DHT.h>
#include "Utils.h"

static const char *TAG = "Sensor";
static DHT s_dht(DHT_PIN, DHT_TYPE);

static SensorReadings s_readings;
static uint32_t s_flameHighSince[FLAME_SENSOR_COUNT] = {0, 0, 0, 0, 0};
static uint32_t s_lastDhtReadMs = 0;
static uint32_t s_lastSmokeReadMs = 0;

static int readSmokeFiltered() {
  uint32_t sum = 0;
  const uint8_t samples = 4;
  for (uint8_t i = 0; i < samples; i++) {
    sum += analogRead(MQ2_PIN);
  }
  return static_cast<int>(sum / samples);
}

static void updateFlameChannels() {
  bool anyDetected = false;
  uint32_t now = millis();
  for (uint8_t i = 0; i < FLAME_SENSOR_COUNT; i++) {
    bool raw = digitalRead(FLAME_PINS[i]) == HIGH;
    if (raw) {
      if (s_flameHighSince[i] == 0) s_flameHighSince[i] = now;
      bool sustained = (now - s_flameHighSince[i]) >= Cfg::FLAME_TRIGGER_MS;
      s_readings.flameChannel[i] = sustained;
    } else {
      s_flameHighSince[i] = 0;
      s_readings.flameChannel[i] = false;
    }
    anyDetected |= s_readings.flameChannel[i];
  }
  s_readings.flameDetected = anyDetected;
}

namespace SensorManager {

void begin() {
  for (uint8_t i = 0; i < FLAME_SENSOR_COUNT; i++) {
    pinMode(FLAME_PINS[i], INPUT);
  }
  pinMode(MQ2_PIN, INPUT);
  s_dht.begin();
  s_readings.smokeRaw = readSmokeFiltered();
  LOG_I(TAG, "SensorManager initialized");
}

void tick() {
  updateFlameChannels();

  if (everyMs(s_lastSmokeReadMs, 100)) {
    s_readings.smokeRaw = readSmokeFiltered();
  }

  if (everyMs(s_lastDhtReadMs, Cfg::DHT_READ_PERIOD_MS)) {
    float t = s_dht.readTemperature();
    float h = s_dht.readHumidity();
    if (!isnan(t) && t >= -20.0f && t <= 80.0f) s_readings.temperature = t;
    if (!isnan(h) && h >= 0.0f && h <= 100.0f) s_readings.humidity = h;
  }

  SharedState::setSensorData(s_readings.temperature, s_readings.humidity,
                              s_readings.smokeRaw, s_readings.flameDetected);
}

SensorReadings getReadings() { return s_readings; }

bool isSmokeAboveThreshold() { return s_readings.smokeRaw > Cfg::SMOKE_THRESHOLD; }

bool isTempHigh() {
  return !isnan(s_readings.temperature) && s_readings.temperature > Cfg::TEMP_HIGH_THRESHOLD_C;
}

bool isFlameDetected() { return s_readings.flameDetected; }

bool isSensorFault() {
  bool dhtFailed = isnan(s_readings.temperature) || isnan(s_readings.humidity);
  bool smokeFailed = (s_readings.smokeRaw < 50);
  return dhtFailed && smokeFailed;
}

} 
