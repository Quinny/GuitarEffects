#ifndef COMPRESSOR_PEDAL_H
#define COMPRESSOR_PEDAL_H

// dynamic.hpp needs these to be included.
#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/dynamic.hpp"
#include "q/fx/envelope.hpp"
#include "signal_type.h"

// Compresses the input signal.
class CompressorPedal : public Pedal {
 public:
  CompressorPedal(double attack_seconds, double release_seconds)
      : attack_seconds_(attack_seconds),
        release_seconds_(release_seconds),
        envelope_tracker_(attack_seconds_, release_seconds_,
                          /* sample_rate= */ 44100) {}

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
    info.name = "Compressor";

    info.knobs = {
        PedalKnob{
            .name = "attack", .value = attack_seconds_, .tweak_amount = 0.1},
        PedalKnob{
            .name = "release", .value = release_seconds_, .tweak_amount = 0.1},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "attack") {
      attack_seconds_ = pedal_knob.value;
    } else if (pedal_knob.name == "release") {
      release_seconds_ = pedal_knob.value;
    }

    envelope_tracker_ =
        cycfi::q::envelope_follower(attack_seconds_, release_seconds_,
                                    /* sample_rate= */ 44100);
  }

 private:
  double attack_seconds_;
  double release_seconds_;
  cycfi::q::compressor compressor_{/* threshold= */ 0.1,
                                   /* ratio= */ 0.1};
  cycfi::q::envelope_follower envelope_tracker_;
};

REGISTER_PEDAL("Compressor", []() {
  return std::unique_ptr<Pedal>(new CompressorPedal(
      /* attack_seconds= */ 0.4, /* release_seconds= */ 0.4));
});

#endif /* COMPRESSOR_PEDAL_H */
