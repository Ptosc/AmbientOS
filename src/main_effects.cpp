#include "hardware.h"

void render_off() {
  fill_solid(leds, NUMPIXELS, CRGB::Black);
}

void render_distance(int distance_cm) {
  int d = distance_cm;
  if (d < 0) d = 80;
  d = constrain(d, 0, 80);

  uint8_t r = map(d, 0, 80, 255, 0);
  uint8_t g = map(d, 0, 80, 0, 255);
  uint8_t b = map(d, 0, 80, 0, 200);

  fill_solid(leds, NUMPIXELS, CRGB(r, g, b));
}

static const CRGB FOCUS_COLORS[] = {
  CRGB(40, 120, 255),
  CRGB(255, 150, 25),
  CRGB(125, 70, 255),
  CRGB(255, 70, 70),
};

void render_focus(const LightMod& m) {
  uint8_t idx = m.color_mode % 4;
  fill_solid(leds, NUMPIXELS, FOCUS_COLORS[idx]);
}

void render_rainbow(const LightMod& m) {
  static float hue = 0.0f;

  float speed = 0.05f + m.energy * 0.03f;
  uint8_t brightness = 60 + (uint8_t)constrain(m.energy * 180.0f, 0.0f, 180.0f);
  uint32_t t = millis();

  for (int i = 0; i < NUMPIXELS; i++) {
    int8_t offset = (int8_t)(inoise8(i * 9, t / 120) - 128) / 6;
    uint8_t h = (uint8_t)(hue + i * 2 + offset);
    leds[i] = CHSV(h, 220, brightness);
  }

  hue += speed;
  if (hue >= 255.0f) hue -= 255.0f;
}
