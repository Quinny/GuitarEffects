#ifndef COMPRESSOR_PEDAL_H
#define COMPRESSOR_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

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
