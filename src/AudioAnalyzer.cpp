#include "AudioAnalyzer.h"

#include <M5Unified.h>
#include <arduinoFFT.h>
#include <math.h>

bool AudioAnalyzer::begin() {
  // Match the mic sample rate to our analysis rate so record() returns frames
  // at exactly kSampleRate.
  auto cfg = M5.Mic.config();
  cfg.sample_rate = kSampleRate;
  cfg.stereo = false;
  M5.Mic.config(cfg);
  return M5.Mic.begin();
}

AudioFeatures AudioAnalyzer::capture() {
  AudioFeatures f;

  // --- 1. Record one frame (blocking) ---------------------------------------
  // record() queues the buffer; isRecording() stays true until it is filled.
  M5.Mic.record(rawBuffer_, kFftSize, kSampleRate);
  while (M5.Mic.isRecording()) {
    delay(1);
  }

  // --- 2. RMS + load FFT input ----------------------------------------------
  // Normalize 16-bit samples to roughly -1..1 and remove DC offset.
  double sum = 0.0;
  for (uint16_t i = 0; i < kFftSize; ++i) {
    sum += rawBuffer_[i];
  }
  const float mean = static_cast<float>(sum / kFftSize);

  double sumSq = 0.0;
  for (uint16_t i = 0; i < kFftSize; ++i) {
    const float s = (rawBuffer_[i] - mean) / 32768.0f;
    vReal_[i] = s;
    vImag_[i] = 0.0f;
    sumSq += static_cast<double>(s) * s;
  }
  f.rms = sqrtf(static_cast<float>(sumSq / kFftSize));

  // --- 3. FFT ---------------------------------------------------------------
  ArduinoFFT<float> fft(vReal_, vImag_, kFftSize, static_cast<float>(kSampleRate));
  fft.windowing(FFTWindow::Hamming, FFTDirection::Forward);
  fft.compute(FFTDirection::Forward);
  fft.complexToMagnitude();  // magnitudes now live in vReal_[0..kFftSize/2]

  // --- 4. Band energies + spectral centroid ---------------------------------
  const float binHz = static_cast<float>(kSampleRate) / kFftSize;
  const uint16_t lowBin = static_cast<uint16_t>(kLowBandHz / binHz);
  const uint16_t midBin = static_cast<uint16_t>(kMidBandHz / binHz);
  const uint16_t maxBin = kFftSize / 2;

  float low = 0.0f, mid = 0.0f, high = 0.0f;
  float weighted = 0.0f, total = 0.0f;
  for (uint16_t bin = 2; bin < maxBin; ++bin) {  // skip bins 0/1 (DC + rumble)
    const float mag = vReal_[bin];
    const float hz = bin * binHz;
    if (bin < lowBin) {
      low += mag;
    } else if (bin < midBin) {
      mid += mag;
    } else {
      high += mag;
    }
    weighted += hz * mag;
    total += mag;
  }

  f.lowEnergy = low;
  f.midEnergy = mid;
  f.highEnergy = high;
  f.centroid = (total > 1e-6f) ? (weighted / total) : 0.0f;
  return f;
}
