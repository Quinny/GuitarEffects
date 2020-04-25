#ifndef DELAY_PEDAL_H
#define DELAY_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

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
    delay_buffer_[delay_index_] = signal;
    delay_index_ = (delay_index_ + 1) % delay_buffer_.size();
    return signal + (delay_buffer_[delay_index_] / 4);
  }

  std::string Describe() override {
    return "delay (" + std::to_string(delay_buffer_.size()) + ")";
  }

 private:
  std::vector<SignalType> delay_buffer_;
  int delay_index_ = 0;
};

REGISTER_PEDAL("delay", []() {
  int delay_frames;
  std::cout << "How many frames would you like to delay? ";
  std::cin >> delay_frames;

  return std::unique_ptr<Pedal>(new DelayPedal(delay_frames));
});

#endif /* DELAY_PEDAL_H */
