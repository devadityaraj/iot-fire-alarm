#pragma once
#include "Config.h"

namespace AlarmManager {
void begin();
void tick();
bool isAlarmActive();
AlertType currentAlertType();
}  // namespace AlarmManager
