#pragma once

#include <M5Unified.h>
#include "Viseme.h"

// MouthRenderer draws a single, as-realistic-as-practical mouth (no eyes).
//
// The mouth is rendered column by column (a vertical scanline per screen x),
// which lets the lip contours, the cupid's bow, and the lip/teeth/tongue
// shading all be smooth. Each viseme maps to target parameters; every frame the
// current parameters are eased toward the target (kSmoothing) so the mouth
// glides between shapes. Drawing happens on an off-screen PSRAM sprite and is
// pushed in one go, which removes flicker.
class MouthRenderer {
 public:
  // Easing factor per frame: new = old + (target - old) * kSmoothing.
  // Lower = calmer/slower, higher = snappier.
  static constexpr float kSmoothing = 0.40f;

  bool begin();
  void setTarget(Viseme v);
  void render();

 private:
  // Mutable description of a mouth shape. All values are normalized 0..1.
  // Plain aggregate (no default member initializers) so brace init works
  // under gnu++11.
  struct MouthShape {
    float open;   // vertical opening of the aperture
    float width;  // horizontal stretch
    float teeth;  // how much the upper teeth show
  };

  static MouthShape targetFor(Viseme v);
  void drawFace();

  M5Canvas canvas_{&M5.Display};
  MouthShape current_{};
  MouthShape target_{};
};
