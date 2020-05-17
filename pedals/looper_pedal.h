#ifndef LOOPER_PEDAL_H
#define LOOPER_PEDAL_H

#include "pedal_registry.h"
#include "signal_type.h"

#include <string>
#include <vector>

// Records input signal and then re-loops it back continually.
//
// The loop blend controls the volume at which the loop is combined with the
// input signal. 1.0 means replay the loop at full volume at which it was
// recorded, 0.8 means 80%, etc.
class LooperPedal : public Pedal {
 public:
  enum class Mode {
    BYPASS,
    RECORD,
    REPLAY,
  };

  SignalType Transform(SignalType signal) override {
    switch (mode_) {
      case Mode::BYPASS:
        return signal;
      case Mode::RECORD:
        loop_buffer_.push_back(signal);
        return signal;
      case Mode::REPLAY:
        SignalType output =
            signal + (loop_buffer_[loop_position_] * loop_blend_);
        loop_position_ = (loop_position_ + 1) % loop_buffer_.size();
        return output;
    }
    return signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Looper";

    info.knobs = {
        PedalKnob{.name = "bypass",
                  .value = static_cast<double>(mode_ == Mode::BYPASS),
                  .tweak_amount = 1},
        PedalKnob{.name = "record",
                  .value = static_cast<double>(mode_ == Mode::RECORD),
                  .tweak_amount = 1},
        PedalKnob{.name = "replay",
                  .value = static_cast<double>(mode_ == Mode::REPLAY),
                  .tweak_amount = 1},
        PedalKnob{
            .name = "loop_blend", .value = loop_blend_, .tweak_amount = 0.1},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "bypass") {
      mode_ = Mode::BYPASS;
      loop_buffer_.clear();
      loop_position_ = 0;
    } else if (pedal_knob.name == "record") {
      mode_ = Mode::RECORD;
      loop_buffer_.clear();
      loop_position_ = 0;
    } else if (pedal_knob.name == "replay") {
      mode_ = Mode::REPLAY;
      loop_position_ = 0;
    } else if (pedal_knob.name == "loop_blend") {
      loop_blend_ = pedal_knob.value;
    }
  }

 private:
  int loop_position_ = 0;
  double loop_blend_ = 1;
  std::vector<SignalType> loop_buffer_;
  Mode mode_ = Mode::BYPASS;
};

REGISTER_PEDAL("Looper",
               []() { return std::unique_ptr<Pedal>(new LooperPedal()); });

#endif /* LOOPER_PEDAL_H */
