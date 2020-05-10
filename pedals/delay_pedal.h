#ifndef DELAY_PEDAL_H
#define DELAY_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

#include <iostream>
#include <vector>

// A pedal which produces a "delay" effect. This creates a buffer of size
// `buffer_size` and adds the input from `buffer_size` frames ago to the current
// input. The "delayed" signal is decayed slightly to prevent it from
// overpowering the actual input.
class DelayPedal : public Pedal {
 public:
  DelayPedal(int buffer_size) : delay_buffer_(buffer_size, 0) {}

  SignalType Transform(SignalType signal) override {
    static constexpr int kDecayFactor = 4;
    delay_buffer_[delay_index_] = signal;
    delay_index_ = (delay_index_ + 1) % delay_buffer_.size();
    return signal + (delay_buffer_[delay_index_] / kDecayFactor);
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Delay";

    info.knobs.push_back(
        PedalKnob{.name = "frames", .value = (double)delay_buffer_.size()});

    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {
    delay_buffer_ = std::vector<SignalType>((int)knob.value, 0);
  }

 private:
  std::vector<SignalType> delay_buffer_;
  int delay_index_ = 0;
};

REGISTER_PEDAL("Delay",
               []() { return std::unique_ptr<Pedal>(new DelayPedal(20000)); });

#endif /* DELAY_PEDAL_H */
