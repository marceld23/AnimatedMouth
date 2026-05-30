# AnimatedMouth — Audio-Reactive Talking Mouth for M5Stack Core2

> **A fun project**: it shows an animated mouth that reacts to sound on the
> M5Stack Core2. Talk (or play music) near it and the mouth opens, closes, and
> changes shape to match what it hears.

A live, audio-reactive animated mouth rendered on the M5Stack Core2 display.
The device listens through its built-in PDM microphone, analyzes the audio in
real time, and drives a cartoon mouth that opens, closes, and changes shape to
match what it "hears".

> **This is not speech recognition.** The Core2 does not try to understand
> words or transcribe speech. Instead it does cheap, real-time **audio analysis**
> and maps the result to a small set of **visemes** (visible mouth shapes). This
> is exactly how most simple "talking avatar" systems work, and it looks
> surprisingly lifelike for a toy / robot face / digital pet.

```
Microphone ─▶ Audio frames ─▶ Feature extraction ─▶ Viseme classifier ─▶ Mouth render
 (PDM, 16kHz)   (~32 ms)        (RMS + FFT bands)     (rule based)         (TFT, smoothed)
```

## Hardware

- **M5Stack Core2** (ESP32-D0WDQ6-V3, 16 MB flash, 8 MB PSRAM)
- 320 × 240 ILI9342C TFT display
- Built-in PDM microphone (analog input)

No extra wiring required — everything used here is on-board.

## What it does

The microphone signal is sliced into short frames (~32 ms). For each frame the
firmware computes:

- **RMS / loudness** — how loud is it right now?
- **Spectral band energy** — low / mid / high frequency energy via FFT
- **Spectral balance** — is the sound dark (O/U), bright (I/E), or hissy (S/Sch/F)?

These features are passed through a small set of rules that pick one of the
mouth shapes (visemes). The chosen shape is then **smoothly interpolated** on
screen so the mouth animates fluidly instead of flickering.

### Visemes

| Viseme        | Trigger (rough)                          | Mouth shape          |
| ------------- | ---------------------------------------- | -------------------- |
| `CLOSED`      | RMS below silence threshold              | thin closed line     |
| `OPEN_A`      | loud, balanced spectrum                  | wide open (A / Ä)    |
| `ROUND_OU`    | low-frequency energy dominates           | small round (O / U)  |
| `WIDE_IE`     | high-frequency energy, not hissy         | wide stretched (I/E) |
| `TEETH_S`     | strong high-frequency noise              | narrow, teeth (S/F)  |
| `SMALL`       | quiet but above silence                  | slightly open        |

## Controls & settings

The mouth view is deliberately clean — no on-screen buttons. The three Core2
touch buttons below the display are used like this:

| Where        | Button       | Action                                  |
| ------------ | ------------ | --------------------------------------- |
| Mouth view   | **B** (middle) | Open the settings menu                |
| Settings     | **A**        | Decrease the selected value             |
| Settings     | **C**        | Increase the selected value             |
| Settings     | **B** (short)| Move to the next setting                |
| Settings     | **B** (hold) | Save & return to the mouth              |

The settings menu lets you adjust:

- **Sensitivity** — volume gain applied before classification. Higher = the
  mouth reacts to quieter sounds.
- **Brightness** — display backlight (applied live as you change it).

It also shows the current **battery level**. Settings are stored in NVS
(ESP32 Preferences), so they survive a reboot.

## Build & flash

This is a [PlatformIO](https://platformio.org/) project.

```bash
# Build
pio run

# Upload to the connected Core2
pio run --target upload

# Open the serial monitor (115200 baud) to see debug feature output
pio device monitor
```

In VS Code with the PlatformIO extension you can also just use the
*Build* / *Upload* / *Monitor* buttons in the status bar.

### Dependencies

Declared in [platformio.ini](platformio.ini) and fetched automatically:

- [`m5stack/M5Unified`](https://github.com/m5stack/M5Unified) — display, mic, board init
- [`kosme/arduinoFFT`](https://github.com/kosme/arduinoFFT) — frequency-band analysis

## Tuning

The classifier thresholds live in [src/VisemeClassifier.cpp](src/VisemeClassifier.cpp)
and the audio frame settings in [src/AudioAnalyzer.h](src/AudioAnalyzer.h).
The most useful knobs:

- `SILENCE_RMS` — how quiet counts as "not speaking"
- band split frequencies (low / mid / high)
- the smoothing factor in [src/MouthRenderer.cpp](src/MouthRenderer.cpp)
  (`0.75 * old + 0.25 * new` by default) — lower = snappier, higher = calmer

## Project layout

```
src/
  main.cpp            orchestration: mode switch + loop
  AudioAnalyzer.*     mic capture + RMS + FFT band features
  Viseme.h           Viseme enum + AudioFeatures struct
  VisemeClassifier.* rule-based feature → viseme mapping
  MouthRenderer.*     parametric mouth drawing + smoothing (sprite, flicker-free)
  Settings.*          user settings + NVS persistence
  SettingsMenu.*      touch-button settings screen (sensitivity, brightness, battery)
```

## License

Released under the [MIT License](LICENSE.md) — © 2026 Marcel Dütscher.
