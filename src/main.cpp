#include <M5Unified.h>

#include "AudioAnalyzer.h"
#include "MouthRenderer.h"
#include "Settings.h"
#include "SettingsMenu.h"
#include "VisemeClassifier.h"

// AnimatedMouth — audio-reactive talking mouth for the M5Stack Core2.
// Pipeline: mic -> features -> viseme -> animated mouth. See README.md.
//
// The mouth view is intentionally button-free. A short press on the middle
// touch button (BtnB) opens the settings menu (volume sensitivity, display
// brightness, battery level); hold BtnB there to save & return.

AudioAnalyzer analyzer;
VisemeClassifier classifier;
MouthRenderer mouth;
Settings settings;
SettingsMenu menu;

enum class Mode { Mouth, Settings };
Mode mode = Mode::Mouth;

uint32_t lastDebugMs = 0;

static void fatal(const char* msg) {
  M5.Display.fillScreen(TFT_RED);
  M5.Display.setTextColor(TFT_WHITE);
  M5.Display.setTextSize(2);
  M5.Display.drawCenterString(msg, 160, 110);
  while (true) {
    delay(1000);
  }
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);

  settings.load();
  M5.Display.setBrightness(settings.brightness);

  if (!analyzer.begin()) fatal("Mic init failed");
  if (!mouth.begin()) fatal("Canvas alloc failed");

  Serial.println("AnimatedMouth ready.");
}

void loop() {
  M5.update();

  if (mode == Mode::Settings) {
    if (!menu.update()) {
      settings.save();
      mode = Mode::Mouth;
    }
    return;
  }

  // --- Mouth mode ----------------------------------------------------------
  // Short press on the middle touch button opens settings.
  if (M5.BtnB.wasClicked()) {
    mode = Mode::Settings;
    menu.open(&settings);
    return;
  }

  // 1. Capture + analyze one ~32 ms audio frame (blocks while recording).
  AudioFeatures features = analyzer.capture();

  // 2. Apply the user's volume sensitivity (gain on the features). Ratios are
  //    preserved, so only the loudness gating shifts, not the vowel shape.
  features.rms *= settings.sensitivity;
  features.lowEnergy *= settings.sensitivity;
  features.midEnergy *= settings.sensitivity;
  features.highEnergy *= settings.sensitivity;

  // 3. Pick the target mouth shape and ease toward it.
  const Viseme viseme = classifier.classify(features);
  mouth.setTarget(viseme);
  mouth.render();

  // 4. Occasional debug output for tuning thresholds.
  const uint32_t now = millis();
  if (now - lastDebugMs >= 1000) {
    lastDebugMs = now;
    Serial.printf("rms=%.4f low=%.1f mid=%.1f high=%.1f centroid=%.0fHz v=%u sens=%.2f\n",
                  features.rms, features.lowEnergy, features.midEnergy,
                  features.highEnergy, features.centroid,
                  static_cast<unsigned>(viseme), settings.sensitivity);
  }
}
