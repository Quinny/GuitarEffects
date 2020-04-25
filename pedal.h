#ifndef PEDAL_H
#define PEDAL_H

#include "signal.h"

// The base pedal class which defines some transformation on the input signal.
//
// `signal` will be a floating point value in [-1, 1]. The result produced by
// the pedal should also conform to that range, otherwise the sound card may
// produce crackling noises.
class Pedal {
 public:
  virtual SignalType Transform(SignalType signal) = 0;
  virtual std::string Describe() = 0;
  virtual ~Pedal() = default;
};

#endif /* PEDAL_H */
