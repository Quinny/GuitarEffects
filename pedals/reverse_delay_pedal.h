#ifndef REVERSE_DELAY_PEDAL_H
#define REVERSE_DELAY_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

#include <iostream>
#include <vector>

class ReverseDelayPedal : public Pedal {
public:
  ReverseDelayPedal(double delay_seconds, double delay_blend)
      : delay_seconds_(delay_seconds), delay_blend_(delay_blend),
        delay_buffer_(delay_seconds_ * 2 * 44100, 0),
        delay_write_index_(0) {}

  SignalType Transform(SignalType signal) override {
    delay_buffer_[delay_write_index_] = signal;
    int delay_read_index = delay_buffer_.size() - delay_write_index_ - 1;
    delay_write_index_ = (delay_write_index_ + 1) % delay_buffer_.size();
    

    SignalType output = ((1 - delay_blend_) * signal) +
                        (delay_buffer_[delay_read_index] * delay_blend_);
    return output;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "ReverseDelay";

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

    delay_buffer_ = std::vector<SignalType>(delay_seconds_ * 2 * 44100, 0);
    delay_write_index_ = 0;
  }

private:
  double delay_seconds_;
  double delay_blend_;
  int delay_write_index_ = 0;
  std::vector<SignalType> delay_buffer_;
};

REGISTER_PEDAL("ReverseDelay", []() {
  return std::unique_ptr<Pedal>(
      new ReverseDelayPedal(/* delay_seconds =*/0.8, /* delay_blend= */ 1.0));
});

#endif /* REVERSE_DELAY_PEDAL_H */
