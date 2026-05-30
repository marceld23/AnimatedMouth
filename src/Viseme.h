#pragma once

#include <stdint.h>

// A "viseme" is a visible mouth shape. We do not recognize speech; we just map
// cheap audio features to one of these coarse shapes (see README).
enum class Viseme : uint8_t {
  Closed,   // silence -> mouth shut
  Small,    // quiet but audible -> slightly open
  OpenA,    // loud, balanced spectrum -> wide open (A / Ae)
  RoundOU,  // low frequencies dominate -> small round (O / U)
  WideIE,   // high frequencies, not hissy -> stretched wide (I / E)
  TeethS,   // strong high-frequency noise -> narrow, teeth (S / Sch / F)
};

// Per-frame audio features extracted by AudioAnalyzer and consumed by the
// VisemeClassifier. All energies are linear (not dB) and only meaningful
// relative to each other.
struct AudioFeatures {
  float rms = 0.0f;       // overall loudness, 0..~1
  float lowEnergy = 0.0f; // band energy, low frequencies (vowels O/U body)
  float midEnergy = 0.0f; // band energy, mid frequencies (A/E body)
  float highEnergy = 0.0f;// band energy, high frequencies (I/E, sibilants)
  float centroid = 0.0f;  // spectral centroid in Hz (brightness)
};
