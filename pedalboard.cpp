#include <algorithm>
#include <cmath>
#include <iostream>
#include <memory>

#include "pedal.h"
#include "pedals/blues_drive_pedal.h"
#include "pedals/compressor_pedal.h"
#include "pedals/delay_pedal.h"
#include "pedals/echo_pedal.h"
#include "pedals/fuzz_pedal.h"

#include "audio_transformer.h"
#include "pedal_registry.h"
#include "playback.h"
#include "signal_type.h"

#include "rtaudio/RtAudio.h"

class PedalChain : public Pedal {
 public:
  using iterator = std::vector<std::unique_ptr<Pedal>>::iterator;

  SignalType Transform(SignalType signal) override {
    std::lock_guard<std::mutex> lock(mu_);
    for (const auto &pedal : pedals_) {
      signal = pedal->Transform(signal);
    }
    // TODO(quinn): Might need to clamp this to prevent it from crackling.
    return signal;
  }

  PedalInfo Describe() override {
    std::lock_guard<std::mutex> lock(mu_);
    PedalInfo info;
    for (auto &pedal : pedals_) {
      info.name += pedal->Describe().name + "-->";
    }
    info.name += "amp";
    return info;
  }

  PedalChain &AddPedal(std::unique_ptr<Pedal> pedal) {
    std::lock_guard<std::mutex> lock(mu_);
    pedals_.push_back(std::move(pedal));
    return *this;
  }

  iterator begin() { return pedals_.begin(); }
  iterator end() { return pedals_.end(); }

  void insert(iterator position, std::unique_ptr<Pedal> pedal) {
    std::lock_guard<std::mutex> lock(mu_);
    pedals_.insert(position, std::move(pedal));
  }

  void remove(iterator position) {
    std::lock_guard<std::mutex> lock(mu_);
    pedals_.erase(position);
  }

 private:
  std::vector<std::unique_ptr<Pedal>> pedals_;
  std::mutex mu_;
};

int main() {
  PedalChain pedal_chain;

  Playback pb(/* filename = */ "recording");
  AudioTransformer at(
      [&pb, &pedal_chain](SignalType input) {
        return pedal_chain.Transform(pb.next());
        // return pedal_chain.Transform(input);
      },
      /* input_device_index= */ 3, /* output_device_index= */ 1,
      /* treat_input_as_mono= */ false);
  at.DumpDeviceInfo();
  at.Start();

  while (1) {
    std::string command;
    std::getline(std::cin, command);
    if (command.empty()) continue;

    std::stringstream tokenizer(command);
    std::vector<std::string> tokens;
    for (std::string token; tokenizer >> token; /* nothing */) {
      tokens.push_back(std::move(token));
    }

    if (tokens[0] == "add") {
      const auto &pedal_type = tokens[1];
      auto pedal_location = tokens.size() > 2
                                ? pedal_chain.begin() + std::stoi(tokens[2])
                                : pedal_chain.end();
      auto *pedal_factory_fn =
          PedalRegistry::GetInstance().GetPedalFactoryOrNull(pedal_type);
      if (pedal_factory_fn) {
        auto pedal = (*pedal_factory_fn)();
        pedal_chain.insert(pedal_location, std::move(pedal));
        std::cout << pedal_chain.Describe().name << std::endl;
      } else {
        std::cout << "Unknown pedal type: " << pedal_type << std::endl;
      }
    } else if (tokens[0] == "remove") {
      auto pedal_location = pedal_chain.begin() + std::stoi(tokens[1]);
      pedal_chain.remove(pedal_location);
      std::cout << pedal_chain.Describe().name << std::endl;
    }
  }
  return 0;
}
