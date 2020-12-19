#ifndef AUTO_WAH_PEDAL_H
#define AUTO_WAH_PEDAL_H

#include "q/support/base.hpp"
#include "q/support/frequency.hpp"
#include "q/support/literals.hpp"

#include "pedal.h"
#include "pedal_registry.h"
#include "q/fx/biquad.hpp"
#include "q/fx/envelope.hpp"

class AutoWahPedal : public Pedal {
public:
  AutoWahPedal() {
    // Force computation of the frequency increment right away.
    AdjustKnob(PedalKnob{});
  }

  SignalType Transform(SignalType signal) override {
    auto env = envelope_tracker_(std::abs(signal));

    // Sweep the filter cutoff proportional to the envelope (volume) of the
    // playing.
    auto cut_off = min_frequency_ +
                   (max_frequency_ - min_frequency_) * (env * responsiveness_);
    lowpass_.config(cut_off, 44100, q_);
    bandpass_.config(cut_off, 44100, q_);

    // Mix the output of the filters with the dry signal.
    auto filter_signal = bandpass_(lowpass_(signal));
    return (filter_mix_ * filter_signal) + (1 - filter_mix_) * signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "AutoWah";

    info.knobs = {
        PedalKnob{
            .name = "max_frequency_hz",
            .value = max_frequency_,
            .tweak_amount = 100,
        },
        PedalKnob{
            .name = "min_frequency_hz",
            .value = min_frequency_,
            .tweak_amount = 100,
        },
        PedalKnob{
            .name = "responsiveness",
            .value = responsiveness_,
            .tweak_amount = 0.1,
        },
        PedalKnob{
            .name = "q",
            .value = q_,
            .tweak_amount = 0.1,
        },
        PedalKnob{
            .name = "filter_mix",
            .value = filter_mix_,
            .tweak_amount = 0.1,
        },
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {
    if (knob.name == "max_frequency_hz") {
      max_frequency_ = knob.value;
    } else if (knob.name == "min_frequency_hz") {
      min_frequency_ = knob.value;
    } else if (knob.name == "responsiveness") {
      responsiveness_ = knob.value;
    } else if (knob.name == "q") {
      q_ = knob.value;
    } else if (knob.name == "filter_mix") {
      filter_mix_ = knob.value;
    }

    lowpass_ = {max_frequency_, 44100, q_};
    bandpass_ = {max_frequency_, 44100, q_};
  }

  cycfi::q::lowpass lowpass_{3000, 44100, 1.0};
  cycfi::q::bandpass_csg bandpass_{3000, 44100, 1.0};

  // Set attack and release very low so they respond quickly to playing
  // dynamics.
  cycfi::q::envelope_follower envelope_tracker_{/* attack_seconds= */ 0.015,
                                                /* release_seconds= */ 0.015,
                                                /* sample_rate= */ 44100};

  double max_frequency_ = 2000;
  double min_frequency_ = 20;
  double responsiveness_ = 2.5;
  double q_ = 2.5;
  double filter_mix_ = 1.0;
};

REGISTER_PEDAL("AutoWah",
               []() { return std::unique_ptr<Pedal>(new AutoWahPedal()); });

#endif /* AUTO_WAH_PEDAL_H */
