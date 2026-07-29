#include "TaskManager.h"

void setup() {
  Serial.begin(115200);
  delay(1000);
  TaskManager::begin();
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
