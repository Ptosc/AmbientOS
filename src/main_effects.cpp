#include "hardware.h"
#include <math.h>

void render_off() {
  fill_solid(leds, NUMPIXELS, CRGB::Black);
}

void render_aurora(const LightMod& m) {
  const uint32_t t = millis() / 30;
  const uint8_t base_val = 70 + (uint8_t)constrain(m.energy * 100.0f, 0.0f, 100.0f);

  for (int i = 0; i < NUMPIXELS; i++) {
    uint8_t wave = inoise8(i * 12, t);
    uint8_t flicker = inoise8(i * 6, t * 2);

    uint8_t hue = 88 + scale8(wave, 72);       // teal -> blue-violet
    uint8_t sat = 180 + scale8(flicker, 60);
    uint8_t val = base_val + scale8(wave, 50);

    leds[i] = CHSV(hue, sat, val);
  }
}

void render_focus(const LightMod& m, FocusPhase phase) {
  (void)m;

  if (phase == FOCUS_ARRIVAL) {
    float breath = (sinf((millis() / 6000.0f) * 6.28318f) + 1.0f) * 0.5f;
    uint8_t val = 100 + (uint8_t)(breath * 100);
    CRGB orange(255, 90, 15);
    orange.nscale8_video(val);
    fill_solid(leds, NUMPIXELS, orange);
    return;
  }

  fill_solid(leds, NUMPIXELS, CRGB(40, 100, 255));
}

void render_visual_state_to(CRGB* buf, const VisualState& vs) {
  if (!vs.active || vs.mode == MODE_OFF) {
    fill_solid(buf, NUMPIXELS, CRGB::Black);
    return;
  }

  switch (vs.mode) {
    case MODE_FOCUS:
      render_focus(mod, vs.focus_phase);
      break;
    case MODE_AURORA:
      render_aurora(mod);
      break;
    case MODE_RAINBOW:
      render_rainbow(mod);
      break;
    default:
      render_off();
      break;
  }

  for (int i = 0; i < NUMPIXELS; i++) buf[i] = leds[i];
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
