#include "hardware.h"

struct ZoneEffect {
    uint8_t intensity = 0;   // 0–255
    uint8_t decay = 20;      // wie schnell er verschwindet
    CRGB color = CRGB::White;
};

ZoneEffect zones[3];

void trigger_zone(uint8_t zone, CRGB color) {
    zones[zone].color = color;
    zones[zone].intensity = 255;
}

void update_status() {
    // 1. jede Zone rendern
    for (int z = 0; z < 3; z++) {

        uint8_t intensity = zones[z].intensity;
        CRGB base = zones[z].color;

        int start = z * 2;
        int end   = start + 2;

        for (int i = start; i < end; i++) {

            // weiche Mitte (LED näher an Zentrum heller)
            uint8_t distance_falloff = (i == start || i == end - 1) ? 180 : 255;

            CRGB c = base;

            // // Puls-Effekt
            // c.nscale8_video(intensity);

            // // Zonen-LED Feintuning
            // c.nscale8_video(distance_falloff);

            status_led[i] = c;
        }

        // decay (smooth fade out)
        zones[z].intensity = qsub8(zones[z].intensity, zones[z].decay);
    }
}