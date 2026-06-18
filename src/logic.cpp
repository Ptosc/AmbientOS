#include "hardware.h"

void handle_mode_buttons(ButtonEvent e) {
  static unsigned long last_mode_change_ms = 0;
  const unsigned long MODE_DEBOUNCE_MS = 250;

  switch (e) {
    case BUTTON_T1:
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
