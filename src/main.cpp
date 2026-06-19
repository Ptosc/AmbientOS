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

static void apply_brightness(uint8_t brightness) {
  if (brightness >= 255) return;
  for (int i = 0; i < NUMPIXELS; i++) {
    leds[i].nscale8_video(brightness);
  }
}

void loop() {
  poll_inputs();
  handle_mode_buttons(poll_buttons());

  update_state();
  update_focus_session();
  compute_modulation();

  update_status();

  const bool active = (presence > 0.02f) || always_on;
  const VisualState target = {mode, focus_phase, active};

  transition_begin_if_changed(target);

  if (transition_is_active()) {
    render_visual_state_to(new_frame, target);
    transition_output(leds);
  } else {
    render_visual_state_to(leds, target);
  }

  apply_brightness(mod.brightness);

  static unsigned long last_debug = 0;
  if (millis() - last_debug >= 250) {
    last_debug = millis();
    VisualState displayed = transition_get_displayed_state();
    VisualState tstate = transition_get_target_state();
    Serial.printf(
      "\n========== DEBUG ==========\n"
      "Time        : %lu ms\n"
      "Mode        : %d\n"
      "Target      : mode=%d phase=%d active=%d\n"
      "Displayed   : mode=%d phase=%d active=%d\n"
      "Transition  : %s (%lu/%lu ms)\n"
      "Presence    : %d %%\n"
      "Distance    : %d cm\n"
      "Raw Dist    : %d cm\n"
      "Brightness  : %d\n"
      "ColorMode   : %d\n"
      "FocusPhase  : %d\n"
      "Encoder 1   : %ld\n"
      "Encoder 2   : %ld\n"
      "===========================\n",
      millis(),
      mode,
      tstate.mode, (int)tstate.focus_phase, tstate.active ? 1 : 0,
      displayed.mode, (int)displayed.focus_phase, displayed.active ? 1 : 0,
      transition_is_active() ? "YES" : "NO",
      transition_elapsed_ms(),
      TRANSITION_MS,
      (int)(presence * 100.0f),
      filtered_distance,
      raw_distance,
      mod.brightness,
      mod.color_mode,
      (int)focus_phase,
      get_encoder1_pos(),
      get_encoder2_pos());
  }

  FastLED.show();
  delay(1);
  yield();
}
