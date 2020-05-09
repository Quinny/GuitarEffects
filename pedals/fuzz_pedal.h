#ifndef FUZZ_PEDAL_H
#define FUZZ_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

// A pedal which creates a fuzz effect by applying hard clipping to the
// input signal.
class FuzzPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    constexpr static SignalType kMax = 0.9;
    constexpr static SignalType kPreGain = 0.4;

    int input_sign = signal > 0 ? 1 : -1;
    SignalType pre_gained = signal + (input_sign * kPreGain);

    if (pre_gained < -kMax) return -kMax;
    if (pre_gained > kMax) return kMax;
    return signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "fuzz";
    return info;
  }
};

REGISTER_PEDAL("fuzz", []() { return std::unique_ptr<Pedal>(new FuzzPedal); });

#endif /* FUZZ_PEDAL_H */
