#ifndef LOWPASS_PEDAL_H
#define LOWPASS_PEDAL_H

#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "pedal.h"
#include "pedal_registry.h"
#include "pedals/filter_pedal.h"
#include "q/fx/biquad.hpp"

// A pedal which applies a basic lowpass filter to the input.
class LowpassPedal : public FilterPedal<cycfi::q::lowpass> {
 public:
  std::string GetName() override { return "LowpassFilter"; }
};

REGISTER_PEDAL("LowpassFilter",
               []() { return std::unique_ptr<Pedal>(new LowpassPedal()); });

#endif /* LOWPASS_PEDAL_H */
