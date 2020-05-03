#ifndef EFFECTS_PIPELINE_H
#define EFFECTS_PIPELINE_H

#include <functional>
#include <vector>
#include "signal.h"

// Utility for applying a pipeline of effects to a signal.
class EffectsPipeline {
 public:
  using Effect = std::function<SignalType(SignalType)>;
  EffectsPipeline(std::vector<Effect> pipeline)
      : pipeline_(std::move(pipeline)) {}

  SignalType operator()(SignalType input) {
    for (const auto& effect : pipeline_) {
      input = effect(input);
    }
    return input;
  }

 private:
  std::vector<Effect> pipeline_;
};

#endif /* EFFECTS_PIPELINE_H */
