#ifndef SAWTOOTH_PEDAL_H
#define SAWTOOTH_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

#include <iostream>
#include <vector>

// A pedal which produces a saw tooth like wave form.
//
// https://en.wikipedia.org/wiki/Sawtooth_wave
class SawtoothPedal : public Pedal {
public:
  SignalType Transform(SignalType signal) override {
    // If the signal is on an "upward" trajectory let it continue, otherwise
    // slam it all the way down.
    SignalType ret = std::abs(signal) > std::abs(last_frame_) ? signal : 0;
    last_frame_ = signal;
    return ret;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Sawtooth";
    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {}

private:
  SignalType last_frame_ = 0;
};

REGISTER_PEDAL("Sawtooth",
               []() { return std::unique_ptr<Pedal>(new SawtoothPedal()); });

#endif /* SAWTOOTH_PEDAL_H */
