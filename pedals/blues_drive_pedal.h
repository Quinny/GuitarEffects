#ifndef BLUES_DRIVE_PEDAL_H
#define BLUES_DRIVE_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/biquad.hpp"
#include "signal.h"

class BluesDrivePedal : public Pedal {
 public:
  SignalType Transform(SignalType signal) override {
    SignalType bandpass = bandpass_(signal);
    SignalType shaped = std::tanh(bandpass);
    SignalType lowpass = lowpass_(shaped);
    return lowpass;
  }

  std::string Describe() override { return "blues drive"; }

 private:
  static constexpr int kSampleRate = 44100;
  static constexpr cycfi::q::frequency kFrequency = 1 * 1e3;

  cycfi::q::bandpass_csg bandpass_{kFrequency, kSampleRate};
  cycfi::q::lowpass lowpass_{kFrequency, kSampleRate};
};

constexpr cycfi::q::frequency BluesDrivePedal::kFrequency;

REGISTER_PEDAL("bluesdrive",
               []() { return std::unique_ptr<Pedal>(new BluesDrivePedal()); });

#endif /* BLUES_DRIVE_PEDAL_H */
