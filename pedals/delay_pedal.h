#ifndef DELAY_PEDAL_H
#define DELAY_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

#include <iostream>
#include <vector>

// A pedal which produces a "delay" effect. This creates a ring buffer
// which holds `delay_seconds` frames and adds the input from `delay_seconds`
// frames ago to the current input. The "delayed" signal is decayed slightly to
// prevent it from overpowering the actual input.
class DelayPedal : public Pedal {
 public:
  DelayPedal(double delay_seconds, double delay_blend)
      : delay_seconds_(delay_seconds),
        delay_blend_(delay_blend),
        delay_buffer_(delay_seconds_ * 44100, 0) {}

  SignalType Transform(SignalType signal) override {
    delay_buffer_[delay_index_] = signal;
    delay_index_ = (delay_index_ + 1) % delay_buffer_.size();
    return signal + (delay_buffer_[delay_index_] * delay_blend_);
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Delay";

    info.knobs = {
        PedalKnob{
            .name = "seconds", .value = delay_seconds_, .tweak_amount = 0.1},
        PedalKnob{
            .name = "delay_blend", .value = delay_blend_, .tweak_amount = 0.1}};

    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {
    if (knob.name == "seconds") {
      delay_seconds_ = knob.value;
    } else if (knob.name == "delay_blend") {
      delay_blend_ = knob.value;
    }

    // Reset the buffer always so that the new settings take effect right away.
    delay_index_ = 0;
    delay_buffer_ = std::vector<SignalType>(delay_seconds_ * 44100, 0);
  }

 private:
  double delay_seconds_;
  double delay_blend_;
  int delay_index_ = 0;
  std::vector<SignalType> delay_buffer_;
};

REGISTER_PEDAL("Delay", []() {
  return std::unique_ptr<Pedal>(
      new DelayPedal(/* delay_seconds =*/0.5, /* delay_blend= */ 0.2));
});

#endif /* DELAY_PEDAL_H */
