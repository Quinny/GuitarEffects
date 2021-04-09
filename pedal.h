#ifndef PEDAL_H
#define PEDAL_H

#include <string>
#include <vector>

#include "signal_type.h"

struct PedalKnob {
  std::string name;
  double value;
  double tweak_amount;
};

struct PedalInfo {
  std::string name;
  std::vector<PedalKnob> knobs;
};

// The base pedal class which defines some transformation on the input signal.
//
// `signal` will be a floating point value in [-1, 1]. The result produced by
// the pedal should also conform to that range, otherwise the sound card may
// produce crackling noises.
class Pedal {
public:
  virtual SignalType Transform(SignalType signal) = 0;
  virtual void AdjustKnob(const PedalKnob& /* knob */) {}
  virtual PedalInfo Describe() = 0;
  virtual ~Pedal() = default;
  virtual void Push() { enabled_ = !enabled_; }
  bool Enabled() const { return enabled_; }

private:
  bool enabled_ = true;
};

#endif /* PEDAL_H */
