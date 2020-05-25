#ifndef DISTORTION_PEDAL_H
#define DISTORTION_PEDAL_H

// biquad.h actually needs these to be imported and I'm to lazy to fork
// cycfi's repo and actually submit these changes.
#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "fx/effects_pipeline.h"
#include "fx/wave_shaper.h"
#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/biquad.hpp"
#include "signal_type.h"

class DistortionPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override { return pipeline_(signal); }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Distortion";

    info.knobs = {
        PedalKnob{.name = "drive", .value = drive_, .tweak_amount = 0.1},
        PedalKnob{
            .name = "frequency_hz", .value = frequency_hz_, .tweak_amount = 50},
        PedalKnob{.name = "curve_sample",
                  .value = curve_sample_,
                  .tweak_amount = 100},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "drive") {
      drive_ = pedal_knob.value;
    } else if (pedal_knob.name == "frequency_hz") {
      frequency_hz_ = pedal_knob.value;
    } else if (pedal_knob.name == "curve_sample") {
      curve_sample_ = pedal_knob.value;
    }

    bandpass_ = {frequency_hz_, kSampleRate};
    lowpass_ = {frequency_hz_, kSampleRate};
    auto curve = [this](int x) { return NonLinearity(x); };
    wave_shaper = {curve, static_cast<int>(curve_sample_)};

    pipeline_ = {{bandpass_, wave_shaper, lowpass_}};
  }

 private:
  SignalType NonLinearity(int x) { return (1 / (1 + std::abs(x))) * drive_; }

  static constexpr int kSampleRate = 44100;
  double frequency_hz_ = 1000;
  double drive_ = 0.2;
  double curve_sample_ = 4096;

  cycfi::q::bandpass_csg bandpass_{frequency_hz_, kSampleRate};
  cycfi::q::lowpass lowpass_{frequency_hz_, kSampleRate};
  WaveShaper wave_shaper{[this](int x) { return NonLinearity(x); },
                         static_cast<int>(curve_sample_)};

  EffectsPipeline pipeline_{{bandpass_, wave_shaper, lowpass_}};
};

REGISTER_PEDAL("Distortion",
               []() { return std::unique_ptr<Pedal>(new DistortionPedal()); });

#endif /* DISTORTION_PEDAL_H */
