#include "../hardware.h"
#include <Arduino.h>

// Implementation of mmWave moved into inputs/ to tidy project layout.
// Exposes init_mmwave_impl() and read_mmwave_impl(); root-level mmwave.cpp will forward.
// NOTE: distances are published as centimeters (cm)

// Configuration defaults
static const int MMWAVE_BAUD = 115200;
static const int MMWAVE_RX_PIN = 34;
static const int MMWAVE_TX_PIN = 16;
static const float MMW_ALPHA = 0.2f;
// thresholds in CENTIMETERS (modifiable at runtime)
static int _mmw_thresh_on = 120;   // cm
static int _mmw_thresh_off = 180;  // cm
static unsigned long _mmw_debounce_ms = 800;

static HardwareSerial mmSerial(2);

// parser buffer
static char line_buf[128];
static size_t line_idx = 0;

// state (centimeters)
static int raw_range_cm = -1;
static float filtered_range_cm = 0.0f;
static bool mm_active = false;
static unsigned long mm_crossed_since = 0;
static bool _last_mm_active = false; // for reduced debug

void set_mmwave_thresholds(int on_cm, int off_cm) {
  _mmw_thresh_on = on_cm;
  _mmw_thresh_off = off_cm;
#ifdef MMWAVE_DEBUG
  Serial.printf("[MMW] thresholds set: on=%dcm off=%dcm\n", _mmw_thresh_on, _mmw_thresh_off);
#endif
}

void set_mmwave_debounce(unsigned long ms) {
  _mmw_debounce_ms = ms;
#ifdef MMWAVE_DEBUG
  Serial.printf("[MMW] debounce set: %lums\n", _mmw_debounce_ms);
#endif
}

int get_mmwave_threshold_on_cm() { return _mmw_thresh_on; }
int get_mmwave_threshold_off_cm() { return _mmw_thresh_off; }
unsigned long get_mmwave_debounce_ms() { return _mmw_debounce_ms; }

void init_mmwave_impl() {
  mmSerial.begin(MMWAVE_BAUD, SERIAL_8N1, MMWAVE_RX_PIN, MMWAVE_TX_PIN);
#ifdef MMWAVE_DEBUG
  Serial.println("[MMW] UART started (impl) - units: cm");
#endif
}

static void process_line(const char *s) {
  // Try to parse an integer value from the line. The sensor may report mm or cm;
  // detect a number and convert mm->cm if value seems large (>500) else keep as cm.
  // This makes parser tolerant to different firmware variants.
  const char *p = s;
  // find first digit or '-' sign
  while (*p && !((*p >= '0' && *p <= '9') || *p == '-')) p++;
  if (!*p) return;
  long v = atol(p);
  if (v == 0) return;
  // Heuristic: if value looks like millimeters (>500) convert to cm
  int cm = (v > 500) ? (int)((v + 5) / 10) : (int)v;
  raw_range_cm = cm;
  if (filtered_range_cm <= 0.0f) filtered_range_cm = (float)raw_range_cm;
  filtered_range_cm = (MMW_ALPHA * raw_range_cm) + ((1.0f - MMW_ALPHA) * filtered_range_cm);
}

void read_mmwave_impl() {
  while (mmSerial.available()) {
    char c = (char)mmSerial.read();
    if (c == '\r') continue;
    if (c == '\n' || line_idx >= sizeof(line_buf)-1) {
      if (line_idx > 0) {
        line_buf[line_idx] = '\0';
        process_line(line_buf);
      }
      line_idx = 0;
    } else {
      if (isPrintable(c)) line_buf[line_idx++] = c;
    }
  }

  unsigned long now = millis();
  bool in_range = (filtered_range_cm > 0.0f) && (filtered_range_cm <= _mmw_thresh_on);
  bool out_range = (filtered_range_cm >= _mmw_thresh_off);

  if (!mm_active) {
    if (in_range) {
      if (mm_crossed_since == 0) mm_crossed_since = now;
      if ((now - mm_crossed_since) >= _mmw_debounce_ms) {
        mm_active = true;
        mm_crossed_since = now;
      }
    } else {
      mm_crossed_since = 0;
    }
  } else {
    if (out_range) {
      if ((now - mm_crossed_since) >= _mmw_debounce_ms) {
        mm_active = false;
        mm_crossed_since = now;
      }
    }
  }

  // publish to globals in CENTIMETERS
  raw_distance = raw_range_cm;
  filtered_distance = (raw_range_cm > 0) ? (int)(filtered_range_cm + 0.5f) : -1;
  sensor_presence = mm_active ? 1.0f : 0.0f;

  // reduced debug: print only on presence change
#ifdef MMWAVE_DEBUG
  if (mm_active != _last_mm_active) {
    _last_mm_active = mm_active;
    Serial.printf("[MMW] presence %s at %dcm (filt=%.1fcm)\n", mm_active ? "ON" : "OFF", raw_distance, filtered_range_cm);
    if (mm_active) Serial.println("[MMW] PRESENCE DETECTED: ********************************************************");
  }
#endif
}
