#include "hardware.h"

struct CandlePixel {
  uint8_t s;
  uint8_t s_target;
  uint8_t fade_step;
};

static CandlePixel candle_pixels[NUMPIXELS];
static uint32_t last_tick_ms = 0;

static unsigned candle_speed_factor(uint8_t speed) {
  if (speed > 252) return 1;
  if (speed > 99) return 2;
  if (speed > 49) return 3;
  return 4;
}

static void candle_tick(CandlePixel& p, unsigned valrange, unsigned rndval, unsigned speedFactor) {
  if (p.fade_step == 0) {
    p.s = 128;
    p.s_target = 130 + random8(4);
    p.fade_step = 1;
  }

  bool new_target = false;
  if (p.s_target > p.s) {
    p.s = qadd8(p.s, p.fade_step);
    if (p.s >= p.s_target) new_target = true;
  } else {
    p.s = qsub8(p.s, p.fade_step);
    if (p.s <= p.s_target) new_target = true;
  }

  if (new_target) {
    p.s_target = random8(rndval) + random8(rndval);
    if (p.s_target < (rndval >> 1)) {
      p.s_target = (rndval >> 1) + random8(rndval);
    }
    p.s_target += (255 - valrange);

    unsigned dif = (p.s_target > p.s) ? (p.s_target - p.s) : (p.s - p.s_target);
    p.fade_step = dif >> speedFactor;
    if (p.fade_step == 0) p.fade_step = 1;
  }
}

void focus_candle_reset() {
  for (int i = 0; i < NUMPIXELS; i++) {
    candle_pixels[i] = {0, 0, 0};
  }
  last_tick_ms = 0;
}

void render_focus_candle(CRGB* out, CRGB bright, CRGB dim) {
  const uint32_t now = millis();
  if (last_tick_ms == 0 || (now - last_tick_ms) >= CANDLE_FRAME_MS) {
    last_tick_ms = now;

    const unsigned valrange = CANDLE_INTENSITY;
    const unsigned rndval = valrange >> 1;
    const unsigned speed_factor = candle_speed_factor(CANDLE_SPEED);

    for (int i = 0; i < NUMPIXELS; i++) {
      candle_tick(candle_pixels[i], valrange, rndval, speed_factor);
    }
  }

  for (int i = 0; i < NUMPIXELS; i++) {
    out[i] = blend(dim, bright, candle_pixels[i].s);
  }
}

void render_focus_candle(CRGB* out) {
  render_focus_candle(out, FOCUS_CANDLE_BRIGHT, FOCUS_CANDLE_DIM);
}
