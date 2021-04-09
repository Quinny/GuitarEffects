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
      SignalType output = signal + (loop_buffer_[loop_position_] * loop_blend_);

      ++loop_position_;
      if (loop_position_ > end_frame_)
        loop_position_ = start_frame_;

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
        PedalKnob{.name = "start_frame",
                  .value = static_cast<double>(start_frame_),
                  .tweak_amount = 22000},
        PedalKnob{.name = "end_frame",
                  .value = static_cast<double>(end_frame_),
                  .tweak_amount = 22000},
    };
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "bypass") {
      mode_ = Mode::BYPASS;
      loop_buffer_.clear();
      loop_position_ = 0;
      start_frame_ = 0;
      end_frame_ = 0;
    } else if (pedal_knob.name == "record") {
      mode_ = Mode::RECORD;
      loop_buffer_.clear();
      loop_position_ = 0;
      start_frame_ = 0;
      end_frame_ = 0;
    } else if (pedal_knob.name == "replay") {
      mode_ = Mode::REPLAY;
      loop_position_ = 0;
      start_frame_ = 0;
      end_frame_ = loop_buffer_.size();
    } else if (pedal_knob.name == "loop_blend") {
      loop_blend_ = pedal_knob.value;
    } else if (pedal_knob.name == "start_frame") {
      int desired_frame = static_cast<int>(pedal_knob.value);
      // Ensure that:
      //   0 <= start_frame < end_frame < buffer_size
      desired_frame = std::max(desired_frame, 0);
      desired_frame = std::min<int>(desired_frame, loop_buffer_.size());
      desired_frame = std::min(desired_frame, end_frame_);

      start_frame_ = desired_frame;
      loop_position_ = start_frame_;
    } else if (pedal_knob.name == "end_frame") {
      int desired_frame = static_cast<int>(pedal_knob.value);
      // Ensure that:
      //   0 <= start_frame < end_frame < buffer_size
      desired_frame = std::max(desired_frame, 0);
      desired_frame = std::max(desired_frame, start_frame_);
      desired_frame = std::min<int>(desired_frame, loop_buffer_.size());

      end_frame_ = desired_frame;
      loop_position_ = start_frame_;
    }
  }

  // Override the button press impact on the looper pedal to shift states
  // instead of disabling.
  void Push() override {
    PedalKnob adjustment;
    switch (mode_) {
    case Mode::BYPASS:
      adjustment.name = "record";
      break;
    case Mode::RECORD:
      adjustment.name = "replay";
      break;
    case Mode::REPLAY:
      adjustment.name = "bypass";
      break;
    }
    AdjustKnob(adjustment);
  }

private:
  int loop_position_ = 0;
  int start_frame_ = 0;
  int end_frame_ = 0;
  double loop_blend_ = 1;
  std::vector<SignalType> loop_buffer_;
  Mode mode_ = Mode::BYPASS;
};

REGISTER_PEDAL("Looper",
               []() { return std::unique_ptr<Pedal>(new LooperPedal()); });

#endif /* LOOPER_PEDAL_H */
