#include "LEDManager.h"
#include <FastLED.h>
#include "Pins.h"

static CRGB s_leds[NUM_LEDS];
static volatile LEDMode s_mode = LEDMode::BOOT_CONNECTING;
static uint32_t s_modeEnteredMs = 0;

static constexpr uint32_t CONNECT_BLINK_HALF_PERIOD_MS = 150;
static constexpr uint32_t ALARM_BLINK_PERIOD_MS = 250;
static constexpr uint32_t SETUP_RAINBOW_HUE_STEP_MS = 12;

static void fillAll(CRGB color) {
  for (int i = 0; i < NUM_LEDS; i++) s_leds[i] = color;
}

static void renderBootConnecting() {
  fillAll(CRGB::Black);
  uint8_t b = beatsin8(12, 8, Cfg::LED_BRIGHTNESS_HALF);
  CRGB c = CRGB::Blue;
  c.nscale8_video(b);
  s_leds[0] = c;
}

static void renderConnectedBlink() {
  uint32_t elapsed = millis() - s_modeEnteredMs;
  uint32_t phase = elapsed / CONNECT_BLINK_HALF_PERIOD_MS;
  bool on = (phase >= 4) || (phase % 2 == 0);
  CRGB c = CRGB::Green;
  c.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
  fillAll(on ? c : CRGB::Black);
}

static void renderOnlineStable() {
  CRGB c = CRGB::Green;
  c.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
  fillAll(c);
}

static void renderOfflineStable() {
  CRGB c = CRGB::Yellow;
  c.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
  fillAll(c);
}

static void renderAlarmActive() {
  bool on = ((millis() / ALARM_BLINK_PERIOD_MS) % 2) == 0;
  fillAll(on ? CRGB::Red : CRGB::Black);
}

static void renderResetBreathing() {
  uint8_t b = beatsin8(15, 10, Cfg::LED_BRIGHTNESS_HALF);
  CRGB c = CRGB::Blue;
  c.nscale8_video(b);
  fillAll(c);
}

static void renderSetupRainbow() {
  uint8_t hue = (uint8_t)((millis() / SETUP_RAINBOW_HUE_STEP_MS) & 0xFF);
  for (int i = 0; i < NUM_LEDS; i++) {
    s_leds[i] = CHSV((uint8_t)(hue + (i * (256 / NUM_LEDS))), 255, 255);
  }
}

static void renderSetupAmber() {
  CRGB c = CRGB(255, 140, 0);
  c.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
  fillAll(c);
}

// LEDs 0..(NUM_LEDS-3) stay green at 50%; last 2 LEDs show a red effect.
static void applyErrorOverlay(bool blink) {
  CRGB green = CRGB::Green;
  green.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
  for (int i = 0; i < NUM_LEDS - 2; i++) s_leds[i] = green;

  CRGB red = CRGB::Red;
  if (blink) {
    // Fast blink for DB errors.
    bool on = ((millis() / 200) % 2) == 0;
    red.nscale8_video(on ? 255 : 0);
  } else {
    // Slow breathing for auth errors.
    uint8_t b = beatsin8(15, 20, 255);
    red.nscale8_video(b);
  }
  s_leds[NUM_LEDS - 2] = red;
  s_leds[NUM_LEDS - 1] = red;
}

static void updateInternalLedPattern() {
  uint32_t now = millis();
  bool ledState = false;

  switch (s_mode) {
    case LEDMode::OFFLINE_STABLE:
    case LEDMode::BOOT_CONNECTING:
      ledState = ((now / 500) % 2) == 0;  // WiFi error / connecting -> Slow Blink
      break;

    case LEDMode::AUTH_ERROR: {
      uint32_t phase = now % 1500;
      ledState = (phase < 150) || (phase >= 300 && phase < 450);  // Auth Error -> Double Blink
      break;
    }

    case LEDMode::DB_ERROR: {
      uint32_t phase = now % 1500;
      ledState = (phase < 120) || (phase >= 240 && phase < 360) || (phase >= 480 && phase < 600);  // DB Error -> Triple Blink
      break;
    }

    case LEDMode::SENSOR_ERROR:
      ledState = ((now / 150) % 2) == 0;  // Sensor Fault -> Fast Continuous Blink
      break;

    case LEDMode::ONLINE_STABLE:
    case LEDMode::ALARM_ACTIVE:
    case LEDMode::RESET_BREATHING:
    default:
      ledState = true;  // Solid HIGH (ON) for non-error operational states
      break;
  }

  digitalWrite(INTERNAL_LED_PIN, ledState ? HIGH : LOW);
}

// Sensor Fault: Flash strip LEDs 1,2 (indices 0,1) & 7,8 (indices 6,7) RED.
static void renderSensorError() {
  bool on = ((millis() / 300) % 2) == 0;
  fillAll(CRGB::Black);
  if (on) {
    CRGB red = CRGB::Red;
    red.nscale8_video(Cfg::LED_BRIGHTNESS_HALF);
    s_leds[0] = red;  // LED 1
    s_leds[1] = red;  // LED 2
    s_leds[6] = red;  // LED 7
    s_leds[7] = red;  // LED 8
  }
}

namespace LEDManager {

void begin() {
  pinMode(INTERNAL_LED_PIN, OUTPUT);
  digitalWrite(INTERNAL_LED_PIN, HIGH);
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(s_leds, NUM_LEDS);
  FastLED.setBrightness(255);
  FastLED.clear(true);
  s_modeEnteredMs = millis();
}

void setMode(LEDMode mode) {
  if (mode != s_mode) {
    s_mode = mode;
    s_modeEnteredMs = millis();
  }
}

LEDMode getMode() { return s_mode; }

bool isConnectBlinkDone() {
  if (s_mode != LEDMode::CONNECTED_BLINK) return true;
  return (millis() - s_modeEnteredMs) >= (4 * CONNECT_BLINK_HALF_PERIOD_MS);
}

void tick() {
  updateInternalLedPattern();

  switch (s_mode) {
    case LEDMode::BOOT_CONNECTING: renderBootConnecting(); break;
    case LEDMode::CONNECTED_BLINK: renderConnectedBlink(); break;
    case LEDMode::ONLINE_STABLE:   renderOnlineStable();   break;
    case LEDMode::OFFLINE_STABLE:  renderOfflineStable();  break;
    case LEDMode::ALARM_ACTIVE:    renderAlarmActive();    break;
    case LEDMode::RESET_BREATHING: renderResetBreathing(); break;
    case LEDMode::SETUP_RAINBOW:   renderSetupRainbow();   break;
    case LEDMode::SETUP_AMBER:     renderSetupAmber();     break;
    case LEDMode::AUTH_ERROR:      applyErrorOverlay(false); break;  // breathing red
    case LEDMode::DB_ERROR:        applyErrorOverlay(true);  break;  // blinking red
    case LEDMode::SENSOR_ERROR:    renderSensorError();    break;
  }
  FastLED.show();
}

}  // namespace LEDManager
