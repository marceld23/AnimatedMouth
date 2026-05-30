#include "VisemeClassifier.h"

Viseme VisemeClassifier::classify(const AudioFeatures& f) const {
  // Silence -> closed.
  if (f.rms < kSilenceRms) {
    return Viseme::Closed;
  }

  // Quiet but audible -> just barely open. Avoids jumpy big shapes on soft
  // sounds and breaths.
  if (f.rms < kSmallRms) {
    return Viseme::Small;
  }

  const float low = f.lowEnergy;
  const float mid = f.midEnergy;
  const float high = f.highEnergy;

  // Sibilants (S / Sch / F): high-frequency noise clearly dominates.
  if (high > (mid + low) * 1.2f) {
    return Viseme::TeethS;
  }

  // Dark vowels (O / U): low frequencies clearly dominate the highs.
  if (low > high * 1.5f) {
    return Viseme::RoundOU;
  }

  // Bright vowels (I / E): highs beat lows but it is not pure hiss.
  if (high > low) {
    return Viseme::WideIE;
  }

  // Everything else: an open, balanced vowel (A / Ae).
  return Viseme::OpenA;
}
