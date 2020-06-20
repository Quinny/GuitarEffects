#ifndef WAVE_SHAPER_PEDAL_H
#define WAVE_SHAPER_PEDAL_H

#include "fx/wave_shaper.h"
#include "pedal.h"
#include "signal_type.h"

// A base class for pedals which apply a wave shaping operation.
class WaveShaperPedal : public Pedal {
 public:
  virtual std::string GetName() = 0;

  // The curve functon which will be used for the basis of shaping.
  virtual SignalType Curve(int x) = 0;

  SignalType Transform(SignalType input) override {
    return wave_shaper_(input);
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = GetName();

    info.knobs = {
        PedalKnob{.name = "curve_points",
                  .value = static_cast<double>(curve_points_),
                  .tweak_amount = 100},
        PedalKnob{.name = "amplitude_multiplier",
                  .value = amplitude_multiplier_,
                  .tweak_amount = 0.1},
        PedalKnob{.name = "period_multiplier",
                  .value = period_multiplier_,
                  .tweak_amount = 0.1},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "curve_points") {
      curve_points_ = static_cast<int>(pedal_knob.value);
    } else if (pedal_knob.name == "amplitude_multiplier") {
      amplitude_multiplier_ = pedal_knob.value;
    } else if (pedal_knob.name == "period_multiplier") {
      period_multiplier_ = pedal_knob.value;
    }

    Update();
  }

 protected:
  void Update() {
    wave_shaper_ = {[this](int x) {
                      return Curve(x * period_multiplier_) *
                             amplitude_multiplier_;
                    },
                    curve_points_};
  }

 private:
  int curve_points_ = 4096;
  double amplitude_multiplier_ = 0.7;
  double period_multiplier_ = 1;
  WaveShaper wave_shaper_{[](int x) { return x; }, curve_points_};
};

#endif /* WAVE_SHAPER_PEDAL_H */
