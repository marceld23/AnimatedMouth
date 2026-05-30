#pragma once

#include <stdint.h>

// User-adjustable settings, persisted in NVS (ESP32 Preferences) so they
// survive a reboot. Edited via the on-device SettingsMenu (button B).
class Settings {
 public:
  // Volume sensitivity: a gain applied to the audio features before
  // classification. Higher = the mouth reacts to quieter sounds.
  static constexpr float kSensMin = 0.50f;
  static constexpr float kSensMax = 3.00f;
  static constexpr float kSensStep = 0.25f;

  // Display backlight (0..255). Clamped to a minimum so the screen is never
  // fully dark.
  static constexpr uint8_t kBrightMin = 26;
  static constexpr uint8_t kBrightMax = 255;
  static constexpr uint8_t kBrightStep = 26;

  float sensitivity = 1.0f;
  uint8_t brightness = 160;

  void load();  // read from NVS (falls back to defaults)
  void save();  // write to NVS

  void adjustSensitivity(int dir);  // dir = -1 or +1, one step, clamped
  void adjustBrightness(int dir);   // dir = -1 or +1, one step, clamped

  // 0..1 fractions for drawing progress bars.
  float sensitivityFraction() const;
  float brightnessFraction() const;
};
