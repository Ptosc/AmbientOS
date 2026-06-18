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

// Transition state
extern bool transitioning;
extern uint16_t transition_step;
extern uint16_t transition_steps_total;
extern int displayed_mode;
extern int target_mode;

// Sensor state (written by Input layer, read by Logic/Render callers)
extern int raw_distance;
extern int filtered_distance;
extern float presence;
extern float sensor_presence;

// Mode state (mode changed only via handle_mode_buttons)
extern int mode;
extern const int max_modes;
extern bool always_on;

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
void compute_modulation();

// --- Render layer (no input reads, no state mutation) ---
void render_off();
void render_focus(const LightMod& mod);
void render_distance(int distance_cm);
void render_rainbow(const LightMod& mod);
void update_status();
void trigger_zone(uint8_t zone, CRGB color);

// --- Encoder (position read only, IRQ-driven) ---
long get_encoder1_pos();
long get_encoder2_pos();

// --- mmWave config ---
void set_mmwave_thresholds(int on_cm, int off_cm);
void set_mmwave_debounce(unsigned long ms);
int get_mmwave_threshold_on_cm();
int get_mmwave_threshold_off_cm();
unsigned long get_mmwave_debounce_ms();

#define TRANSITION_FRAMES 30

// Internal implementations (src/inputs/)
void init_encoders_impl();
void init_buttons_impl();
void init_mmwave_impl();
void read_mmwave_impl();
long get_encoder1_pos_impl();
long get_encoder2_pos_impl();
uint8_t read_poti_impl(Poti& p);
