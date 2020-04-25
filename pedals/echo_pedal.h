#ifndef ECHO_PEDAL_H
#define ECHO_PEDAL_H

#include <iostream>
#include <vector>

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

class EchoPedal : public Pedal {
 public:
  EchoPedal(int buffer_size) : echo_buffer_(buffer_size, 0) {}

  SignalType Transform(SignalType signal) override {
    echo_buffer_[echo_index_] = (echo_buffer_[echo_index_] + signal) / 4;
    echo_index_ = (echo_index_ + 1) % echo_buffer_.size();
    return signal + echo_buffer_[echo_index_];
  }

  std::string Describe() override {
    return "echo (" + std::to_string(echo_buffer_.size()) + ")";
  }

 private:
  std::vector<SignalType> echo_buffer_;
  int echo_index_ = 0;
};

REGISTER_PEDAL("echo", []() {
  int buffer_size;
  std::cout << "How many frames would you like to echo?";
  std::cin >> buffer_size;
  return std::unique_ptr<Pedal>(new EchoPedal(buffer_size));
});

#endif /* ECHO_PEDAL_H */
