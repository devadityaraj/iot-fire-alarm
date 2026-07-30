#pragma once
#include "Config.h"

namespace LEDManager {
void begin();
void tick();
void setMode(LEDMode mode);
LEDMode getMode();
bool isConnectBlinkDone();
}  
