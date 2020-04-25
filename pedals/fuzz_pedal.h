#ifndef FUZZ_PEDAL_H
#define FUZZ_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

// A pedal which creates a fuzz effect by applying hard clipping to the
// input signal.
class FuzzPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    if (signal < -0.5) return -0.9;
    if (signal > 0.5) return 0.9;
    return signal;
  }

  std::string Describe() override { return "fuzz"; }
};

REGISTER_PEDAL("fuzz", []() { return std::unique_ptr<Pedal>(new FuzzPedal); });

#endif /* FUZZ_PEDAL_H */
