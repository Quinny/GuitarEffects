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
  EchoPedal(int buffer_size) : echo_buffer_(buffer_size, 0) {}

  SignalType Transform(SignalType signal) override {
    static constexpr int kDecayFactor = 4;
    echo_buffer_[echo_index_] =
        (echo_buffer_[echo_index_] + signal) / kDecayFactor;
    echo_index_ = (echo_index_ + 1) % echo_buffer_.size();
    return signal + echo_buffer_[echo_index_];
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "echo";
    return info;
  }

 private:
  std::vector<SignalType> echo_buffer_;
  int echo_index_ = 0;
};

REGISTER_PEDAL("echo",
               []() { return std::unique_ptr<Pedal>(new EchoPedal(20000)); });

#endif /* ECHO_PEDAL_H */
