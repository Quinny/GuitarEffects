#ifndef FUZZ_PEDAL_H
#define FUZZ_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

// A pedal which creates a fuzz effect by applying hard clipping to the
// input signal.
class FuzzPedal : public Pedal {
 public:
  FuzzPedal(double pre_gain) : pre_gain_(pre_gain) {}

  SignalType Transform(SignalType signal) override {
    constexpr static SignalType kMax = 0.9;

    int input_sign = signal > 0 ? 1 : -1;
    SignalType pre_gained = signal + (input_sign * pre_gain_);

    if (pre_gained < -kMax) return -kMax;
    if (pre_gained > kMax) return kMax;
    return signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Fuzz";
    info.knobs = {
        PedalKnob{.name = "pre-gain", .value = pre_gain_, .tweak_amount = 0.1}};
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    pre_gain_ = pedal_knob.value;
  }

 private:
  double pre_gain_;
};

REGISTER_PEDAL("Fuzz", []() {
  return std::unique_ptr<Pedal>(new FuzzPedal(/* pre_gain = */ 0.4));
});

#endif /* FUZZ_PEDAL_H */
