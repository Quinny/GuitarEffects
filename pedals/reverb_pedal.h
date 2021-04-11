#ifndef REVERB_PEDAL_H
#define REVERB_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "pedals/delay_pedal.h"
#include "signal_type.h"

#include <iostream>
#include <vector>

// Reverb based on Schroeder's algoritm: 4 parallel delays with slightly
// different delay times + 2 series all pass filters.
class ReverbPedal : public Pedal {
public:
  ReverbPedal(double delay_seconds, double delay_blend)
      : delay_seconds_(delay_seconds), delay_blend_(delay_blend) {
    AdjustKnob({});
  }

  SignalType Transform(SignalType signal) override {
    SignalType result = 0;
    for (auto& delay : delays_) {
      result += delay.Transform(signal);
    }
    for (auto& allpass : allpasses_) {
      result = allpass(result);
    }
    return result;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Reverb";

    info.knobs = {
        PedalKnob{
            .name = "seconds", .value = delay_seconds_, .tweak_amount = 0.1},
        PedalKnob{
            .name = "delay_blend", .value = delay_blend_, .tweak_amount = 0.1},
        PedalKnob{
            .name = "allpass_hz", .value = allpass_hz_, .tweak_amount = 100},
        PedalKnob{.name = "q", .value = q_, .tweak_amount = 0.1},
    };

    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {
    if (knob.name == "seconds") {
      delay_seconds_ = knob.value;
    } else if (knob.name == "delay_blend") {
      delay_blend_ = knob.value;
    } else if (knob.name == "allpass_hz") {
      allpass_hz_ = knob.value;
    } else if (knob.name == "q") {
      q_ = knob.value;
    }

    delays_.clear();
    delays_.emplace_back(delay_seconds_, delay_blend_);
    delays_.emplace_back(delay_seconds_ - 0.0117, delay_blend_);
    delays_.emplace_back(delay_seconds_ + 0.01931, delay_blend_);
    delays_.emplace_back(delay_seconds_ - 0.00797, delay_blend_);

    allpasses_.clear();
    allpasses_.emplace_back(allpass_hz_, 44100, q_);
    allpasses_.emplace_back(allpass_hz_, 44100, q_);
  }

private:
  double delay_seconds_;
  double delay_blend_;
  double allpass_hz_ = 1200;
  double q_ = 0.7;
  std::vector<DelayPedal> delays_;
  std::vector<cycfi::q::allpass> allpasses_;
};

REGISTER_PEDAL("Reverb", []() {
  return std::unique_ptr<Pedal>(
      new ReverbPedal(/* delay_seconds =*/0.5, /* delay_blend= */ 0.2));
});

#endif /* REVERB_PEDAL_H */
