#pragma once

#include "Viseme.h"

// Rule-based mapping from per-frame audio features to a viseme. Intentionally
// simple and stateless: smoothing happens later in MouthRenderer so the mouth
// does not flicker between shapes.
class VisemeClassifier {
 public:
  // Below this RMS the mouth is considered closed (no one is speaking).
  static constexpr float kSilenceRms = 0.012f;
  // Below this RMS (but above silence) we only open the mouth a little.
  static constexpr float kSmallRms = 0.030f;

  Viseme classify(const AudioFeatures& f) const;
};
