#pragma once

#define FLAME1_PIN 32
#define FLAME2_PIN 33
#define FLAME3_PIN 25
#define FLAME4_PIN 26
#define FLAME5_PIN 27

#define MQ2_PIN 34

#define DHT_PIN 22
#define DHT_TYPE DHT11

#define BUTTON_PIN 13

#define BUZZER_PIN 23

#define LED_PIN 18
#define INTERNAL_LED_PIN 2
#define NUM_LEDS 8

#define FLAME_SENSOR_COUNT 5

static const uint8_t FLAME_PINS[FLAME_SENSOR_COUNT] = {
    FLAME1_PIN, FLAME2_PIN, FLAME3_PIN, FLAME4_PIN, FLAME5_PIN
};
