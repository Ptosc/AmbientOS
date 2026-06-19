#pragma once
#include <Arduino.h>
#include <FastLED.h>

// LED SETUP
#define PIN 5
#define NUMPIXELS 107
#define STATUS_PIN 33
#define STATUS_LED_COUNT 6

#define COLOR_ORDER GRB
#define LED_TYPE WS2812B

// INPUT PINS
extern const int taster1_pin;
extern const int taster2_pin;
extern const int taster3_pin;
extern const int taster_encoder1_pin;
extern const int taster_encoder2_pin;
extern const int mmwave_rx_pin;
extern const int mmwave_tx_pin;
extern const int poti_pin;
extern const int encoder1_a_pin;
extern const int encoder1_b_pin;
extern const int encoder2_a_pin;
extern const int encoder2_b_pin;

// LED BUFFERS
extern CRGB leds[NUMPIXELS];
extern CRGB status_led[STATUS_LED_COUNT];
extern CRGB prev_frame[NUMPIXELS];
extern CRGB new_frame[NUMPIXELS];

// Sensor state (written by Input layer, read by Logic/Render callers)
extern int raw_distance;
extern int filtered_distance;
extern float presence;
extern float sensor_presence;

// Mode state (mode changed only via handle_mode_buttons)
extern int mode;
extern const int max_modes;
static const int MODE_OFF = 0;
static const int MODE_FOCUS = 1;
static const int MODE_AURORA = 2;
static const int MODE_RAINBOW = 3;
extern bool always_on;

// Focus session (mode 1): arrival warmup -> deep focus
enum FocusPhase {
  FOCUS_NONE = 0,
  FOCUS_ARRIVAL,
  FOCUS_DEEP,
};

static const unsigned long FOCUS_WARMUP_MS = 10000;

extern FocusPhase focus_phase;

static const unsigned long TRANSITION_MS = 1100;

struct VisualState {
  int mode;
  FocusPhase focus_phase;
  bool active;
};

// Modulation (computed by Logic layer, passed into Render)
struct LightMod {
  uint8_t color_mode;
  float energy;
  uint8_t brightness;
  float motion;
};

struct Poti {
  int pin;
  float smooth;
};

extern Poti poti;
extern LightMod mod;

// Raw poti sample from poll_inputs()
extern uint8_t g_poti_raw;

// --- Input layer ---
enum ButtonEvent {
  BUTTON_NONE = 0,
  BUTTON_T1,
  BUTTON_T2,
  BUTTON_T3,
};

void init_inputs();
void poll_inputs();
ButtonEvent poll_buttons();

// --- Logic layer ---
void handle_mode_buttons(ButtonEvent e);
void update_state();
void update_focus_session();
void skip_focus_warmup();
void compute_modulation();

// --- Render layer (no input reads, no state mutation) ---
void render_off();
void render_focus(const LightMod& mod, FocusPhase phase);
void render_aurora(const LightMod& m);
void render_rainbow(const LightMod& mod);
void render_visual_state_to(CRGB* buf, const VisualState& vs);
void update_status();
void trigger_zone(uint8_t zone, CRGB color);

// --- Transitions ---
void transition_begin_if_changed(const VisualState& target);
bool transition_is_active();
void transition_output(CRGB* dst);
VisualState transition_get_displayed_state();
VisualState transition_get_target_state();
unsigned long transition_elapsed_ms();

// --- Encoder (position read only, IRQ-driven) ---
long get_encoder1_pos();
long get_encoder2_pos();

// --- mmWave config ---
void set_mmwave_thresholds(int on_cm, int off_cm);
void set_mmwave_debounce(unsigned long ms);
int get_mmwave_threshold_on_cm();
int get_mmwave_threshold_off_cm();
unsigned long get_mmwave_debounce_ms();

// Internal implementations (src/inputs/)
void init_encoders_impl();
void init_buttons_impl();
void init_mmwave_impl();
void read_mmwave_impl();
long get_encoder1_pos_impl();
long get_encoder2_pos_impl();
uint8_t read_poti_impl(Poti& p);
