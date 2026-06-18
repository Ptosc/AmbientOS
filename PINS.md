# Pin‑Map & Wiring — AmbientOS (ESP32)

## Übersicht
- Board: ESP32 (Arduino core)
- Projekt: AmbientOS (LED‑Strip + Ultraschall + Potis + Button)
- Serielle Konsole: 115200 Baud (für Live‑Debug im Serial Monitor)

## Pin‑Map
| GPIO | Symbol / Name      | Rolle                                | Richtung  | Hinweise / Details |
|------:|--------------------|--------------------------------------|-----------|--------------------|
| 5     | PIN                | LED‑Strip Data (WS2812B)             | Ausgang   | FastLED; bei 5V‑Strip Level‑Shifter empfehlen |
| 18    | taster_pin         | Button (Mode cycle)                  | Eingang   | INPUT_PULLUP; Taster zwischen GPIO und GND (active LOW) |
| 19    | taster2_pin        | Button 2                             | Eingang   | INPUT_PULLUP |
| 21    | taster3_pin        | Button 3                             | Eingang   | INPUT_PULLUP |
| 16    | trig_pin           | HC‑SR04 TRIG                         | Ausgang   | 10µs Pulse via digitalWrite/delayMicroseconds |
| 4     | echo_pin           | HC‑SR04 ECHO                         | Eingang   | ECHO ist 5V — unbedingt Pegelwandlung (z.B. Spannungsteiler) ! |
| 32    | poti_pin           | Potentiometer (Intensität)           | ADC1      | ADC1_CH4; Spannungsteilung: 0..3.3V; analogRead -> 0..4095 (zweite Reihe) |
| 25    | encoder1_a_pin     | Rotary Encoder 1 — A (CLK)           | Eingang   | INPUT_PULLUP; interruptfähig, Richtung per B lesen |
| 26    | encoder1_b_pin     | Rotary Encoder 1 — B (DT)            | Eingang   | INPUT_PULLUP |
| 27    | encoder1_btn_pin   | Rotary Encoder 1 — Push Button       | Eingang   | INPUT_PULLUP; active LOW |
| 13    | encoder2_a_pin     | Rotary Encoder 2 — A (CLK)           | Eingang   | INPUT_PULLUP; interruptfähig, Richtung per B lesen |
| 22    | encoder2_b_pin     | Rotary Encoder 2 — B (DT)            | Eingang   | INPUT_PULLUP |
| 14    | encoder2_btn_pin   | Rotary Encoder 2 — Push Button       | Eingang   | INPUT_PULLUP; active LOW |


## Kurz‑Wiring (Beispiele)
- LED‑Strip (WS2812B)
  - Strip 5V -> Versorgung (bei 5V Strip)
  - Strip GND -> ESP32 GND (gemeinsame Masse!)
  - Strip DIN -> GPIO5 (Data)

- HC‑SR04
  - VCC -> 5V (oder 3.3V je nach Modul)
  - GND -> ESP32 GND
  - TRIG -> GPIO16
  - ECHO -> Spannungsteiler (z. B. 1k + 2k) -> GPIO4 (max 3.3V)

- Poti (z. B. 10k)
  - Poti linker Pin -> 3.3V
  - Poti rechter Pin -> GND
  - Mittelkontakt -> GPIO32 (poti_pin)

- Taster (erste Reihe)
  - Mode -> GPIO18
  - Taster2 -> GPIO19
  - Taster3 -> GPIO21
  - GND -> ESP32 GND

- Rotary Encoder 1 (A/B + Push) — zweite Reihe
  - A (CLK) -> GPIO25 (`encoder1_a_pin`)  
  - B (DT)  -> GPIO26 (`encoder1_b_pin`)  
  - SW      -> GPIO27 (`encoder1_btn_pin`) 
  - GND     -> ESP32 GND

- Rotary Encoder 2 (A/B + Push) — zweite Reihe
  - A (CLK) -> GPIO13 (`encoder2_a_pin`)  
  - B (DT)  -> GPIO22 (`encoder2_b_pin`)  
  - SW      -> GPIO14 (`encoder2_btn_pin`) 
  - GND     -> ESP32 GND

## Wichtige Hinweise / Best Practices
- Gemeinsame Masse: Immer GND zwischen ESP32, Sensoren und LED‑Versorgung verbinden.
- ECHO (HC‑SR04) ist häufig 5V — NICHT direkt an ESP32 anschließen. Verwende Spannungsteiler oder Logik‑Level‑Shifter.
- WS2812B Data: Wenn Strip mit 5V betrieben wird, kann ein Level‑Shifter die Zuverlässigkeit verbessern. Einige Strips arbeiten aber direkt mit 3.3V Datenpegel.
- ADC: Verwende ADC1 (GPIO32/33) — ADC2 kann Probleme bei aktiviertem WiFi haben.
- Serielle Konsole: Verwende 115200 Baud (Serial.begin(115200) ist im Code vorhanden).

## Debug / Serial
- Aktiv: Serial Monitor auf 115200 öffnen.
- Der Code schreibt periodisch Debug‑Zeilen (t, mode, transition, distance, presence, intensity).
- Falls du CSV‑ oder JSON‑Output willst, sag Bescheid — passe ich an.

## Dateien im Projekt
- `src/hardware.h` — zentrale Pin‑Definitionen und globale Deklarationen
- `src/main.cpp` — Setup, Loop, Serial‑Debug
- `src/input.cpp` — Button, Ultraschall, Potis
- `src/modulation.cpp` — Potimeter‑Smoothing und Intensity
- `src/effects.cpp` — Render‑Funktionen

## Empfohlene Bauteile
- Logic Level Shifter (bidirektional) oder N‑Kanal MOSFET/74HCTxx für Data‑Level shifting
- HC‑SR04 (Ultraschall) + 1k/2k Spannungsteiler für ECHO
- 10k Potentiometer
- 470µF Kondensator nahe LED‑Strip Versorgung