#include "Settings.h"

#include <Preferences.h>

namespace {
constexpr char kNamespace[] = "animmouth";
constexpr char kKeySens[] = "sens";
constexpr char kKeyBright[] = "bright";

template <typename T>
T clampT(T v, T lo, T hi) {
  return v < lo ? lo : (v > hi ? hi : v);
}
}  // namespace

void Settings::load() {
  Preferences prefs;
  prefs.begin(kNamespace, true);  // read-only
  sensitivity = prefs.getFloat(kKeySens, sensitivity);
  brightness = prefs.getUChar(kKeyBright, brightness);
  prefs.end();

  sensitivity = clampT(sensitivity, kSensMin, kSensMax);
  brightness = clampT<uint8_t>(brightness, kBrightMin, kBrightMax);
}

void Settings::save() {
  Preferences prefs;
  prefs.begin(kNamespace, false);  // read-write
  prefs.putFloat(kKeySens, sensitivity);
  prefs.putUChar(kKeyBright, brightness);
  prefs.end();
}

void Settings::adjustSensitivity(int dir) {
  sensitivity = clampT(sensitivity + dir * kSensStep, kSensMin, kSensMax);
}

void Settings::adjustBrightness(int dir) {
  const int next = static_cast<int>(brightness) + dir * kBrightStep;
  brightness = clampT<uint8_t>(static_cast<uint8_t>(clampT(next, 0, 255)),
                               kBrightMin, kBrightMax);
}

float Settings::sensitivityFraction() const {
  return (sensitivity - kSensMin) / (kSensMax - kSensMin);
}

float Settings::brightnessFraction() const {
  return static_cast<float>(brightness - kBrightMin) /
         static_cast<float>(kBrightMax - kBrightMin);
}
