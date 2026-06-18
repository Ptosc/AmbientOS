#include "hardware.h"

// ===== REAL DEFINITIONS =====

CRGB leds[NUMPIXELS];
CRGB status_led[STATUS_LED_COUNT];
CRGB prev_frame[NUMPIXELS];
CRGB new_frame[NUMPIXELS];

int raw_distance = -1;
int filtered_distance = -1;

float presence = 0.0;
float sensor_presence = 0.0; // from active sensor (mmWave)

int mode = 0;
const int max_modes = 4; // rainbow, warm, distance, off
bool always_on = false;

LightMod mod;

// Define potentiometers in a single place (global definitions)
Poti poti  = {32, 0};

// Transition state
bool transitioning = false;
uint16_t transition_step = 0;
uint16_t transition_steps_total = TRANSITION_FRAMES;
int displayed_mode = 0;
int target_mode = 0;