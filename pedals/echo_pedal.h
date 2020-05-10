#ifndef ECHO_PEDAL_H
#define ECHO_PEDAL_H

#include <iostream>
#include <vector>

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

// Echos the input signal delayed back onto itself, decaying the echo at each
// step.
//
// The difference between echo and delay is that echo propagates the previous
// signal back into the buffer instead of overwriting with the input.
class EchoPedal : public Pedal {
 public:
  EchoPedal(double echo_seconds, double decay_factor)
      : echo_seconds_(echo_seconds),
        decay_factor_(decay_factor),
        echo_buffer_(44100 * echo_seconds_, 0) {}

  SignalType Transform(SignalType signal) override {
    echo_buffer_[echo_index_] =
        (echo_buffer_[echo_index_] + signal) / decay_factor_;
    echo_index_ = (echo_index_ + 1) % echo_buffer_.size();
    return signal + echo_buffer_[echo_index_];
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Echo";
    info.knobs = {
        PedalKnob{.name = "echo_seconds",
                  .value = echo_seconds_,
                  .tweak_amount = 0.1},
        PedalKnob{.name = "decay_factor",
                  .value = decay_factor_,
                  .tweak_amount = 0.5},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "echo_seconds") {
      echo_seconds_ = pedal_knob.value;
    } else if (pedal_knob.name == "decay_factor") {
      decay_factor_ = pedal_knob.value;
    }

    echo_index_ = 0;
    echo_buffer_ = std::vector<SignalType>(44100 * echo_seconds_, 0);
  }

 private:
  double echo_seconds_;
  double decay_factor_;
  int echo_index_ = 0;
  std::vector<SignalType> echo_buffer_;
};

REGISTER_PEDAL("Echo", []() {
  return std::unique_ptr<Pedal>(
      new EchoPedal(/* echo_seconds= */ 0.5, /* decay_factor= */ 4));
});

#endif /* ECHO_PEDAL_H */
