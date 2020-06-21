#ifndef SIGMOID_PEDAL_H
#define SIGMOID_PEDAL_H

#include "pedal_registry.h"
#include "pedals/wave_shaper_pedal.h"

class SigmoidPedal : public WaveShaperPedal {
 public:
  SigmoidPedal() { Update(); }

  std::string GetName() override { return "SigmoidPedal"; }
  SignalType Curve(SignalType x) override { return (x / (1 + std::abs(x))); }
};

REGISTER_PEDAL("SigmoidPedal",
               []() { return std::unique_ptr<Pedal>(new SigmoidPedal()); });

#endif /* SIGMOID_PEDAL_H */
