#ifndef FILTER_PEDAL_H
#define FILTER_PEDAL_H

#include "pedal.h"
#include "signal_type.h"

// A base class for standard filtering pedals (e.g. lowpass, bandpass, etc.).
template <typename FilterType>
class FilterPedal : public Pedal {
 public:
  virtual std::string GetName() = 0;

  SignalType Transform(SignalType input) override { return filter_(input); }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = GetName();

    info.knobs = {
        PedalKnob{
            .name = "cut_off_hz", .value = cut_off_hz_, .tweak_amount = 100},
        PedalKnob{.name = "sample_rate",
                  .value = static_cast<double>(sample_rate_),
                  .tweak_amount = 100}};
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "cut_off_hz") {
      cut_off_hz_ = pedal_knob.value;
    } else if (pedal_knob.name == "sample_rate") {
      sample_rate_ = static_cast<unsigned int>(pedal_knob.value);
    }

    filter_ = {cut_off_hz_, sample_rate_};
  }

 private:
  unsigned int sample_rate_ = 44100;
  double cut_off_hz_ = 1000;
  FilterType filter_{cut_off_hz_, sample_rate_};
};

#endif /* FILTER_PEDAL_H */
