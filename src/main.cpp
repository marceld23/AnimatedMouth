#include <M5Unified.h>

#include "AudioAnalyzer.h"
#include "MouthRenderer.h"
#include "VisemeClassifier.h"

// AnimatedMouth — audio-reactive talking mouth for the M5Stack Core2.
// Pipeline: mic -> features -> viseme -> animated mouth. See README.md.

AudioAnalyzer analyzer;
VisemeClassifier classifier;
MouthRenderer mouth;

// Print one debug line per second so the serial monitor stays readable.
uint32_t lastDebugMs = 0;

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  M5.Display.setRotation(1);
  M5.Display.setBrightness(160);

  if (!analyzer.begin()) {
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawCenterString("Mic init failed", 160, 110);
    while (true) {
      delay(1000);
    }
  }

  if (!mouth.begin()) {
    M5.Display.fillScreen(TFT_RED);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.setTextSize(2);
    M5.Display.drawCenterString("Canvas alloc failed", 160, 110);
    while (true) {
      delay(1000);
    }
  }

  Serial.println("AnimatedMouth ready.");
}

void loop() {
  M5.update();

  // 1. Capture + analyze one ~32 ms audio frame (blocks while recording).
  const AudioFeatures features = analyzer.capture();

  // 2. Pick the target mouth shape.
  const Viseme viseme = classifier.classify(features);
  mouth.setTarget(viseme);

  // 3. Ease the mouth toward the target and redraw.
  mouth.render();

  // 4. Occasional debug output for tuning thresholds.
  const uint32_t now = millis();
  if (now - lastDebugMs >= 1000) {
    lastDebugMs = now;
    Serial.printf("rms=%.4f low=%.1f mid=%.1f high=%.1f centroid=%.0fHz v=%u\n",
                  features.rms, features.lowEnergy, features.midEnergy,
                  features.highEnergy, features.centroid,
                  static_cast<unsigned>(viseme));
  }
}
