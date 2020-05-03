#ifndef EFFECTS_PIPELINE_H
#define EFFECTS_PIPELINE_H

#include <functional>
#include <vector>
#include "signal.h"

// Utility for applying a pipeline of effects to a signal. An "effect" is
// defined as function which takes a signal and returns a signal.
//
// Effects are applied in the order which they appear in the input vector.
class EffectsPipeline {
 public:
  using Effect = std::function<SignalType(SignalType)>;
  EffectsPipeline(std::vector<Effect> pipeline)
      : pipeline_(std::move(pipeline)) {}

  SignalType operator()(SignalType input) const {
    for (const auto& effect : pipeline_) {
      input = effect(input);
    }
    return input;
  }

 private:
  std::vector<Effect> pipeline_;
};

#endif /* EFFECTS_PIPELINE_H */
