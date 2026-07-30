#pragma once
#include "Config.h"

namespace TelegramManager {
void begin();
void tick();
void sendToAll(const String &text);
}  
