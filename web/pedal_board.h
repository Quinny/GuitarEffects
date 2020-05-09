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
      input = pedal->Transform(input);
    }
    return input;
  }

  // TODO: this.
  PedalInfo Describe() override { return PedalInfo{}; }

  std::vector<PedalInfo> GetPedals() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PedalInfo> pedals;
    for (const auto& pedal : pedals_) {
      pedals.push_back(pedal->Describe());
    }
    return pedals;
  }

 private:
  std::mutex mutex_;
  std::vector<std::unique_ptr<Pedal>> pedals_;
};

#endif /* PEDAL_BOARD_H */
