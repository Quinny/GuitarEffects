#ifndef FUZZ_PEDAL_H
#define FUZZ_PEDAL_H

#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "fx/wave_shaper.h"
#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/biquad.hpp"
#include "signal_type.h"

// A pedal which creates a fuzz effect by applying hard clipping to the
// input signal.
class FuzzPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    signal = shaper_(signal);
    signal = bandpass_(signal * frequency_multipler_);
    return signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Fuzz";
    info.knobs = {
        PedalKnob{.name = "boost", .value = boost_, .tweak_amount = 0.1},
        PedalKnob{.name = "frequency_multipler",
                  .value = frequency_multipler_,
                  .tweak_amount = 0.1},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "boost") {
      boost_ = pedal_knob.value;
    } else if (pedal_knob.name == "frequency_multipler") {
      frequency_multipler_ = pedal_knob.value;
    }

    shaper_ = {[this](int x) { return Curve(x); }, 4096};
  }

 private:
  SignalType Curve(int x) {
    auto y = std::tanh(x) * boost_;
    auto clipped = std::max<double>(std::min<double>(y, 1), -1);
    return clipped;
  }

  double boost_ = 0.7;
  double frequency_multipler_ = 2;

  cycfi::q::bandpass_csg bandpass_{1200, 44100};
  WaveShaper shaper_{[this](int x) { return Curve(x); }, 4096};
};

REGISTER_PEDAL("Fuzz",
               []() { return std::unique_ptr<Pedal>(new FuzzPedal()); });

#endif /* FUZZ_PEDAL_H */
