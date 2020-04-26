#ifndef BLUES_DRIVE_PEDAL_H
#define BLUES_DRIVE_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal.h"

class BluesDrivePedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    return signal > 0 ? 1 - std::exp(-signal) : -1 + std::exp(signal);
    /*
    current_ = (current_ + 1) % period_;
    return std::tanh(0.5 * current_ * 3.14159) * signal;*/
  }

  std::string Describe() override { return "blues drive"; }

  int period_ = 100000;
  int current_ = 0;
};

REGISTER_PEDAL("bluesdrive",
               []() { return std::unique_ptr<Pedal>(new BluesDrivePedal()); });

#endif /* BLUES_DRIVE_PEDAL_H */
