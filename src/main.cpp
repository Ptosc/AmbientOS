#include "hardware.h"
#include <Arduino.h>
#include <FastLED.h>

// ===== PIN DEFINITIONS =====

const int taster1_pin = 23;
const int taster2_pin = 18;
const int taster3_pin = 17;
const int taster_encoder1_pin = 19;
const int taster_encoder2_pin = 21;

const int mmwave_tx_pin = 16;
const int mmwave_rx_pin = 34;
const int poti_pin = 32;

const int encoder1_a_pin = 25;
const int encoder1_b_pin = 26;
const int encoder2_a_pin = 13;
const int encoder2_b_pin = 22;

void setup() {
  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUMPIXELS);
  FastLED.addLeds<WS2812B, STATUS_PIN, GRB>(status_led, STATUS_LED_COUNT);
  FastLED.setBrightness(255);
  FastLED.clear(true);

  init_inputs();

  Serial.begin(115200);
  Serial.println("BOOT");
  Serial.setDebugOutput(true);
}

static void capture_current_frame(CRGB* dst) {
  for (int i = 0; i < NUMPIXELS; i++) dst[i] = leds[i];
}

static void render_mode_to_buffer(int m, CRGB* buffer) {
  for (int i = 0; i < NUMPIXELS; i++) leds[i] = CRGB::Black;

  switch (m) {
    case 0: render_off(); break;
    case 1: render_focus(mod); break;
    case 2: render_distance(filtered_distance); break;
    case 3: render_rainbow(mod); break;
    default: render_off(); break;
  }

  for (int i = 0; i < NUMPIXELS; i++) buffer[i] = leds[i];
  for (int i = 0; i < NUMPIXELS; i++) leds[i] = prev_frame[i];
}

static float ease_in_out(float t) {
  return t * t * (3.0f - 2.0f * t);
}

static void blend_frames(CRGB* a, CRGB* b, float t) {
  float te = ease_in_out(constrain(t, 0.0f, 1.0f));
  uint8_t ta = (uint8_t)((1.0f - te) * 255);
  uint8_t tb = (uint8_t)(te * 255);
  for (int i = 0; i < NUMPIXELS; i++) {
    CRGB aa = a[i];
    CRGB bb = b[i];
    uint16_t r = (uint16_t)scale8_video(aa.r, ta) + (uint16_t)scale8_video(bb.r, tb);
    uint16_t g = (uint16_t)scale8_video(aa.g, ta) + (uint16_t)scale8_video(bb.g, tb);
    uint16_t b_ = (uint16_t)scale8_video(aa.b, ta) + (uint16_t)scale8_video(bb.b, tb);
    leds[i].r = (r > 255) ? 255 : (uint8_t)r;
    leds[i].g = (g > 255) ? 255 : (uint8_t)g;
    leds[i].b = (b_ > 255) ? 255 : (uint8_t)b_;
  }
}

void loop() {
  // Input
  poll_inputs();
  handle_mode_buttons(poll_buttons());

  // Logic
  update_state();
  compute_modulation();

  update_status();

  // Transition (only place that sets target_mode besides mode assignment in handle_mode_buttons)
  if (mode != target_mode) {
    capture_current_frame(prev_frame);
    render_mode_to_buffer(mode, new_frame);
    target_mode = mode;
    transitioning = true;
    transition_step = 0;
    transition_steps_total = TRANSITION_FRAMES;
  }

  bool active = (presence > 0.02f) || always_on;
  if (!active) {
    fadeToBlackBy(leds, NUMPIXELS, 10);
    FastLED.show();
    return;
  }

  // Render
  if (transitioning) {
    float t = (float)transition_step / (float)transition_steps_total;
    blend_frames(prev_frame, new_frame, t);
    transition_step++;
    if (transition_step >= transition_steps_total) {
      transitioning = false;
      displayed_mode = target_mode;
    }
  } else {
    switch (mode) {
      case 0: render_off(); break;
      case 1: render_focus(mod); break;
      case 2: render_distance(filtered_distance); break;
      case 3: render_rainbow(mod); break;
      default: render_off(); break;
    }
  }

  if (mod.brightness < 255) {
    for (int i = 0; i < NUMPIXELS; i++) {
      leds[i].nscale8_video(mod.brightness);
    }
  }

  static unsigned long last_debug = 0;
  if (millis() - last_debug >= 250) {
    last_debug = millis();
    Serial.printf(
      "\n========== DEBUG ==========\n"
      "Time        : %lu ms\n"
      "Mode        : %d\n"
      "Target      : %d\n"
      "Displayed   : %d\n"
      "Transition  : %s (%d/%d)\n"
      "Presence    : %d %%\n"
      "Distance    : %d cm\n"
      "Raw Dist    : %d cm\n"
      "Brightness  : %d\n"
      "ColorMode   : %d\n"
      "Encoder 1   : %ld\n"
      "Encoder 2   : %ld\n"
      "===========================\n",
      millis(),
      mode,
      target_mode,
      displayed_mode,
      transitioning ? "YES" : "NO",
      transition_step,
      transition_steps_total,
      (int)(presence * 100.0f),
      filtered_distance,
      raw_distance,
      mod.brightness,
      mod.color_mode,
      get_encoder1_pos(),
      get_encoder2_pos());
  }

  FastLED.show();
  delay(1);
  yield();
}
