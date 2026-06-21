#include "hardware.h"

static unsigned long focus_deep_at = 0;
static bool focus_phase_manual = false;
static int last_mode = MODE_OFF;
static long last_showcase_enc1 = 0;
static long last_showcase_enc2 = 0;

void update_focus_session() {
  const bool present = user_is_present();
  const bool in_focus = (mode == MODE_FOCUS);

  if (!in_focus || !present) {
    focus_deep_at = 0;
    focus_phase = FOCUS_NONE;
    focus_phase_manual = false;
    focus_candle_reset();
    return;
  }

  if (focus_deep_at == 0) {
    focus_deep_at = millis() + FOCUS_WARMUP_MS;
  }

  if (focus_phase_manual) return;

  focus_phase = (millis() >= focus_deep_at) ? FOCUS_DEEP : FOCUS_ARRIVAL;
}

void skip_focus_warmup() {
  if (mode != MODE_FOCUS || focus_phase != FOCUS_ARRIVAL) return;
  focus_deep_at = millis();
  focus_phase = FOCUS_DEEP;
  focus_phase_manual = true;
}

void toggle_focus_color() {
  if (mode != MODE_FOCUS || focus_phase == FOCUS_NONE) return;
  focus_phase_manual = true;
  focus_phase = (focus_phase == FOCUS_DEEP) ? FOCUS_ARRIVAL : FOCUS_DEEP;
}

static void handle_mode_change() {
  if (mode == last_mode) return;

  if (mode == MODE_SHOWCASE) {
    showcase_reset();
    last_showcase_enc1 = get_encoder1_pos();
    last_showcase_enc2 = get_encoder2_pos();
  }

  if (mode == MODE_FOCUS) {
    const bool already_at_desk = user_is_present();
    focus_phase_manual = false;
    if (already_at_desk) {
      focus_deep_at = millis();
      focus_phase = FOCUS_DEEP;
      focus_candle_reset();
    } else {
      focus_deep_at = 0;
      focus_phase = FOCUS_NONE;
    }
  }

  last_mode = mode;
}

void update_showcase_inputs() {
  if (mode != MODE_SHOWCASE) return;

  const long enc1 = get_encoder1_pos();
  const long enc2 = get_encoder2_pos();
  const int16_t d1 = (int16_t)(enc1 - last_showcase_enc1);
  const int16_t d2 = (int16_t)(enc2 - last_showcase_enc2);

  showcase_update(d1, d2, millis());

  last_showcase_enc1 = enc1;
  last_showcase_enc2 = enc2;
}

static const unsigned long T2_DOUBLE_CLICK_MS = 400;
static unsigned long t2_last_click_ms = 0;
static bool t2_pending_single = false;

static void cycle_light_mode() {
  if (mode == MODE_OFF || mode == MODE_SHOWCASE) {
    mode = MODE_FOCUS;
  } else {
    mode = MODE_SHOWCASE;
  }
  handle_mode_change();
}

static bool handle_t2_click() {
  const unsigned long now = millis();

  if (t2_pending_single && (now - t2_last_click_ms) <= T2_DOUBLE_CLICK_MS) {
    t2_pending_single = false;
    mode = MODE_OFF;
    handle_mode_change();
    trigger_status_shutdown();
    return true;
  }

  t2_pending_single = true;
  t2_last_click_ms = now;
  return false;
}

void update_mode_button_pending() {
  if (!t2_pending_single) return;
  if (millis() - t2_last_click_ms <= T2_DOUBLE_CLICK_MS) return;

  t2_pending_single = false;
  cycle_light_mode();
}

void handle_mode_buttons(ButtonEvent e) {
  switch (e) {
    case BUTTON_T1:
      skip_focus_warmup();
      trigger_zone(0, CRGB::Red);
      break;

    case BUTTON_T2:
      if (!handle_t2_click()) {
        trigger_zone(1, CRGB::Green);
      }
      break;

    case BUTTON_T3:
      if (mode == MODE_FOCUS) {
        toggle_focus_color();
      }
      trigger_zone(2, CRGB::Blue);
      break;

    default:
      break;
  }
}
