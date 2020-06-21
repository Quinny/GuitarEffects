#ifndef TANH_PEDAL_H
#define TANH_PEDAL_H

#include "pedal_registry.h"
#include "pedals/wave_shaper_pedal.h"

class TanhPedal : public WaveShaperPedal {
 public:
  TanhPedal() { Update(); }

  std::string GetName() override { return "TanhPedal"; }
  SignalType Curve(SignalType x) override { return std::tanh(x); }
};

REGISTER_PEDAL("TanhPedal",
               []() { return std::unique_ptr<Pedal>(new TanhPedal()); });

#endif /* TANH_PEDAL_H */
