#ifndef BLUES_DRIVE_PEDAL_H
#define BLUES_DRIVE_PEDAL_H

#include "fx/effects_pipeline.h"
#include "fx/wave_shaper.h"
#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/biquad.hpp"
#include "signal.h"

class BluesDrivePedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override { return pipeline_(signal); }

  std::string Describe() override { return "blues drive"; }

 private:
  static constexpr int kSampleRate = 44100;
  static constexpr cycfi::q::frequency kFrequency = 1 * 1e3;

  cycfi::q::bandpass_csg bandpass_{kFrequency, kSampleRate};
  cycfi::q::lowpass lowpass_{kFrequency, kSampleRate};
  std::function<SignalType(int)> curve_ = [](int x) { return std::tanh(x); };
  WaveShaper wave_shaper{curve_, 4096};

  EffectsPipeline pipeline_{{bandpass_, wave_shaper, lowpass_}};
};

constexpr cycfi::q::frequency BluesDrivePedal::kFrequency;

REGISTER_PEDAL("bluesdrive",
               []() { return std::unique_ptr<Pedal>(new BluesDrivePedal()); });

#endif /* BLUES_DRIVE_PEDAL_H */
