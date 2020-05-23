#ifndef FUZZ_PEDAL_H
#define FUZZ_PEDAL_H

#include "pedal.h"
#include "pedal_registry.h"
#include "signal_type.h"

// A pedal which creates a fuzz effect by applying hard clipping to the
// input signal.
class FuzzPedal : public Pedal {
 public:
  FuzzPedal() : signal_window_(44100 * window_size_seconds_, 0) {}

  SignalType Transform(SignalType signal) override {
    // Adpatively maintain a clip threshold based on the average signal observed
    // over the duration of the window. This removes the need to tune the fuzz
    // to work with the gain level of the input.

    // Remove the last sample from the window.
    window_sum_ -= signal_window_[window_end_];
    // Add the current sample.
    signal_window_[window_end_] = std::abs(signal);
    window_sum_ += signal_window_[window_end_];
    // Move the window ahead.
    window_end_ = (window_end_ + 1) % signal_window_.size();

    // Select the threshold as the average amplitude over the window.
    SignalType clip_threshold = window_sum_ / signal_window_.size();
    if (signal < -clip_threshold) {
      return -clip_threshold;
    }
    if (signal > clip_threshold) {
      return clip_threshold;
    }

    return signal;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "Fuzz";
    info.knobs = {PedalKnob{.name = "window_size_seconds",
                            .value = window_size_seconds_,
                            .tweak_amount = 0.2}};
    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "window_size_seconds") {
      window_size_seconds_ = pedal_knob.value;
    }

    signal_window_ = std::vector<SignalType>(44100 * window_size_seconds_, 0);
    window_end_ = 0;
    window_sum_ = 0;
  }

 private:
  double window_size_seconds_ = 0.2;
  std::vector<SignalType> signal_window_;
  SignalType window_sum_ = 0;
  int window_end_ = 0;
};

REGISTER_PEDAL("Fuzz",
               []() { return std::unique_ptr<Pedal>(new FuzzPedal()); });

#endif /* FUZZ_PEDAL_H */
