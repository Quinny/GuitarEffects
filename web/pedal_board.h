#ifndef PEDAL_BOARD_H
#define PEDAL_BOARD_H

#include "pedal.h"

#include <mutex>

class PedalBoard : public Pedal {
public:
  void AddPedal(std::unique_ptr<Pedal> pedal) {
    std::lock_guard<std::mutex> lock(mutex_);
    pedals_.push_back(std::move(pedal));
  }

  SignalType Transform(SignalType input) override {
    std::lock_guard<std::mutex> lock(mutex_);
    for (const auto& pedal : pedals_) {
      if (pedal->Enabled()) {
        input = pedal->Transform(input);
      }
    }
    return input;
  }

  void AdjustKnob(int pedal_index, const PedalKnob& knob) {
    std::lock_guard<std::mutex> lock(mutex_);
    pedals_[pedal_index]->AdjustKnob(knob);
  }

  void RemovePedal(int pedal_index) {
    std::lock_guard<std::mutex> lock(mutex_);
    pedals_.erase(pedals_.begin() + pedal_index);
  }

  // TODO: this.
  PedalInfo Describe() override { return PedalInfo{}; }

  void Push(int index) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (index >= 0 && index < pedals_.size()) {
      pedals_[index]->Push();
    }
  }

  std::vector<PedalInfo> GetPedals() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PedalInfo> pedals;
    for (const auto& pedal : pedals_) {
      pedals.push_back(pedal->Describe());
    }
    return pedals;
  }

private:
  mutable std::mutex mutex_;
  std::vector<std::unique_ptr<Pedal>> pedals_;
};

#endif /* PEDAL_BOARD_H */
