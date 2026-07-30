#pragma once
#include "Config.h"

namespace Storage {
void begin();
bool hasConfiguration();
bool loadConfig(DeviceConfig &cfg);
bool saveConfig(const DeviceConfig &cfg);
void eraseAll();
}  // namespace Storage
