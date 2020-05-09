#ifndef COMPRESSOR_PEDAL_H
#define COMPRESSOR_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/dynamic.hpp"
#include "q/fx/envelope.hpp"
#include "signal_type.h"

// Compresses the input signal.
class CompressorPedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    // Take the absolute value of the signal here because we only care about its
    // amplitude, not its sign, when performing compression.
    SignalType compression_gain =
        SignalType(compressor_(envelope_tracker_(std::abs(signal))));

    // TODO: Consider adding post gain here to makeup for volume drop due to
    // compression.
    return signal * compression_gain;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "compressor";
    return info;
  }

 private:
  cycfi::q::compressor compressor_{/* threshold= */ 0.1,
                                   /* ratio= */ 0.1};

  cycfi::q::envelope_follower envelope_tracker_{/* attack= */ 400 * 1e-3,
                                                /* release= */ 400 * 1e-3,
                                                /* sample_rate= */ 44100};
};

REGISTER_PEDAL("compressor",
               []() { return std::unique_ptr<Pedal>(new CompressorPedal()); });

#endif /* COMPRESSOR_PEDAL_H */
