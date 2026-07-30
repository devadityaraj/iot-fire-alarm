#pragma once
#include "Config.h"
#include "Pins.h"

struct SensorReadings {
  float temperature = NAN;
  float humidity = NAN;
  int smokeRaw = 0;
  bool flameDetected = false;
  bool flameChannel[FLAME_SENSOR_COUNT] = {false, false, false, false, false};
};

namespace SensorManager {
void begin();
void tick();
SensorReadings getReadings();
bool isSmokeAboveThreshold();
bool isTempHigh();
bool isFlameDetected();
bool isSmokeWarming();   // true during the 10s gas-sensor warm-up window
bool isSensorFault();
}  // namespace SensorManager
