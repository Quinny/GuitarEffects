#ifndef COMPRESSOR_PEDAL_H
#define COMPRESSOR_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

// Attempts to compress the signal within a specific amplitude range.
//
// Instead of applying hard clipping outside of the range, this pedal does soft
// clipping (allowing the signal to go outside the range, but damping the
// portion that goes outside).
class CompressorPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    constexpr SignalType kThreshold = 0.7;
    auto diff = std::abs(signal) - kThreshold;
    float signal_sign = signal < 0 ? -1 : 1;
    return diff > 0 ? signal_sign * kThreshold + (signal / 10) : signal;
  }

  std::string Describe() override { return "compressor"; }
};

REGISTER_PEDAL("compressor",
               []() { return std::unique_ptr<Pedal>(new CompressorPedal()); });

#endif /* COMPRESSOR_PEDAL_H */
