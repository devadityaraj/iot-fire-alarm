#pragma once
#include "Config.h"

namespace FirebaseManager {
void begin();
bool authenticate();
void tick();
bool isAuthenticated();
}  // namespace FirebaseManager
