#ifndef BANDPASS_PEDAL_H
#define BANDPASS_PEDAL_H

#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "pedal.h"
#include "pedal_registry.h"
#include "pedals/filter_pedal.h"
#include "q/fx/biquad.hpp"

// A pedal which applies a basic bandpass filter to the input.
class BandpassPedal : public FilterPedal<cycfi::q::bandpass_csg> {
 public:
  std::string GetName() override { return "BandpassFilter"; }
};

REGISTER_PEDAL("BandpassFilter",
               []() { return std::unique_ptr<Pedal>(new BandpassPedal()); });

#endif /* BANDPASS_PEDAL_H */
