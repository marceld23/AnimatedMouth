#pragma once

#include <Arduino.h>
#include "Viseme.h"

// AudioAnalyzer captures one frame from the Core2 PDM microphone and turns it
// into a small set of frequency-domain features (see AudioFeatures).
//
// Frame math:
//   sample rate = 16000 Hz
//   FFT size    = 512 samples  ->  32 ms per frame
//   bin width   = 16000 / 512  =  31.25 Hz  (usable up to 8 kHz)
class AudioAnalyzer {
 public:
  static constexpr uint32_t kSampleRate = 16000;
  static constexpr uint16_t kFftSize = 512;  // must be a power of two

  // Band split frequencies (Hz). Tune these to taste.
  static constexpr float kLowBandHz = 500.0f;   // low  : 80 .. 500 Hz
  static constexpr float kMidBandHz = 2000.0f;  // mid  : 500 .. 2000 Hz
                                                // high : 2000 .. 8000 Hz

  // Allocates the FFT buffers and starts the microphone. Returns false if the
  // mic could not be started.
  bool begin();

  // Blocks until one frame (kFftSize samples) is captured, then computes and
  // returns its features. Cheap enough to run every animation tick.
  AudioFeatures capture();

 private:
  int16_t rawBuffer_[kFftSize];
  float vReal_[kFftSize];
  float vImag_[kFftSize];
};
