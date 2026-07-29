#pragma once
#include "Config.h"

namespace WiFiManagerEx {
void begin();
bool connectWithFullRetrySequence();
bool waitForInternet(uint32_t timeoutMs = 8000);
void tick();
bool isConnected();
int8_t getRssi();
String getCurrentSsid();
}  // namespace WiFiManagerEx
