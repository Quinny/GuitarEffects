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

  // The filter which is applied to the signal for modulation. BOTH runs the
  // signal through a lowpass and then a bandpass.
  enum FilterType { LOWPASS = 0, BANDPASS = 1, BOTH = 2 };

  SignalType Transform(SignalType signal) override {
    // Scale the frequency of the modulation with the envelope (volume) of the
    // playing.
    auto env = envelope_tracker_(std::abs(signal));
    current_cut_off_ += frequency_increment_ * ((1 + env) * responsiveness_);

    if (current_cut_off_ > max_frequency_ ||
        current_cut_off_ < min_frequency_) {
      frequency_increment_ = -frequency_increment_;
    }

    lowpass_.config(current_cut_off_, 44100);
    bandpass_.config(current_cut_off_, 44100);

    switch (filter_) {
    case LOWPASS:
      return lowpass_(signal);
    case BANDPASS:
      return bandpass_(signal);
    case BOTH:
      return bandpass_(lowpass_(signal));
    }
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
            .name = "period_length_seconds",
            .value = period_length_seconds_,
            .tweak_amount = 0.1,
        },
        PedalKnob{
            .name = "responsiveness",
            .value = responsiveness_,
            .tweak_amount = 0.1,
        },
        PedalKnob{
            .name = "lowpass",
            .value = static_cast<double>(filter_ == LOWPASS),
            .tweak_amount = 1.0,
        },
        PedalKnob{
            .name = "bandpass",
            .value = static_cast<double>(filter_ == BANDPASS),
            .tweak_amount = 1.0,
        },
        PedalKnob{
            .name = "both",
            .value = static_cast<double>(filter_ == BOTH),
            .tweak_amount = 1.0,
        },
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& knob) override {
    if (knob.name == "max_frequency_hz") {
      max_frequency_ = knob.value;
    } else if (knob.name == "min_frequency_hz") {
      min_frequency_ = knob.value;
    } else if (knob.name == "period_length_seconds") {
      period_length_seconds_ = knob.value;
    } else if (knob.name == "responsiveness") {
      responsiveness_ = knob.value;
    } else if (knob.name == "lowpass") {
      filter_ = LOWPASS;
    } else if (knob.name == "bandpass") {
      filter_ = BANDPASS;
    } else if (knob.name == "both") {
      filter_ = BOTH;
    }

    // min + ((length * 44100) * increment) = max
    // length * 44100 * increment = max - min
    // increment = (max - min) / (length * 44100)
    frequency_increment_ =
        (max_frequency_ - min_frequency_) / (period_length_seconds_ * 44100);
    lowpass_ = {max_frequency_, 44100};
    bandpass_ = {max_frequency_, 44100};
  }

  cycfi::q::lowpass lowpass_{3000, 44100};
  cycfi::q::bandpass_csg bandpass_{3000, 44100};

  // Set attack and release very low so they respond quickly to playing
  // dynamics.
  cycfi::q::envelope_follower envelope_tracker_{/* attack_seconds= */ 0.015,
                                                /* release_seconds= */ 0.015,
                                                /* sample_rate= */ 44100};

  double max_frequency_ = 2000;
  double min_frequency_ = 20;
  double frequency_increment_;
  double period_length_seconds_ = 1.2;
  double current_cut_off_ = min_frequency_;
  double responsiveness_ = 1;
  FilterType filter_ = BOTH;
};

REGISTER_PEDAL("AutoWah",
               []() { return std::unique_ptr<Pedal>(new AutoWahPedal()); });

#endif /* AUTO_WAH_PEDAL_H */
