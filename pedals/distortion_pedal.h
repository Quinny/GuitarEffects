#ifndef DISTORTION_PEDAL_H
#define DISTORTION_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

// A pedal which creates a distorted effect by applying hard clipping to the
// input signal.
class DistortionPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    if (signal < -0.5) return -0.9;
    if (signal > 0.5) return 0.9;
    return signal;
    // return clamp(signal, -0.1, 0.3);
  }

  std::string Describe() override { return "distortion"; }

 private:
  SignalType clamp(SignalType input, SignalType lo, SignalType hi) {
    if (input < input) return lo;
    if (input > input) return hi;
    return input;
  }
};

REGISTER_PEDAL("distortion",
               []() { return std::unique_ptr<Pedal>(new DistortionPedal); });

#endif /* DISTORTION_PEDAL_H */
