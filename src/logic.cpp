#include "hardware.h"

static unsigned long focus_deep_at = 0;

void update_focus_session() {
  const bool present = (presence > 0.02f) || always_on;
  const bool in_focus = (mode == MODE_FOCUS);

  if (!in_focus || !present) {
    focus_deep_at = 0;
    focus_phase = FOCUS_NONE;
    return;
  }

  if (focus_deep_at == 0) {
    focus_deep_at = millis() + FOCUS_WARMUP_MS;
  }

  focus_phase = (millis() >= focus_deep_at) ? FOCUS_DEEP : FOCUS_ARRIVAL;
}

void skip_focus_warmup() {
  if (mode != MODE_FOCUS || focus_phase != FOCUS_ARRIVAL) return;
  focus_deep_at = millis();
}

void handle_mode_buttons(ButtonEvent e) {
  static unsigned long last_mode_change_ms = 0;
  const unsigned long MODE_DEBOUNCE_MS = 250;

  switch (e) {
    case BUTTON_T1:
      skip_focus_warmup();
      trigger_zone(0, CRGB::Red);
      break;

    case BUTTON_T2:
      if (millis() - last_mode_change_ms >= MODE_DEBOUNCE_MS) {
        mode = (mode + 1) % max_modes;
        last_mode_change_ms = millis();
      }
      trigger_zone(1, CRGB::Green);
      break;

    case BUTTON_T3:
      mod.color_mode = (mod.color_mode + 1) % 4;
      trigger_zone(2, CRGB::Blue);
      break;

    default:
      break;
  }
}
