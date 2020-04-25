#include <algorithm>
#include <cmath>
#include <memory>

// Pedals.
#include "blues_drive_pedal.h"
#include "compressor_pedal.h"
#include "delay_pedal.h"
#include "distortion_pedal.h"
#include "echo_pedal.h"
#include "pedal.h"

// Audio engine.
#include "pedal_registry.h"
#include "signal.h"

// External.
#include "rtaudio/RtAudio.h"

// Probe all the connected audio devices.
void probe() {
  RtAudio audio;
  // Determine the number of devices available
  unsigned int devices = audio.getDeviceCount();
  // Scan through devices for various capabilities
  RtAudio::DeviceInfo info;
  for (unsigned int i = 0; i < devices; i++) {
    info = audio.getDeviceInfo(i);
    if (info.probed == true) {
      // Print, for example, the maximum number of output channels for each
      // device
      std::cout << "device ID = " << i << std::endl;
      std::cout << "device name: " << info.name << std::endl;
      std::cout << ": maximum output channels = " << info.outputChannels
                << std::endl;
      std::cout << ": maximum input channels = " << info.inputChannels
                << std::endl;
      std::cout << "Native format: " << info.nativeFormats << std::endl;
    }
  }
}

// Callback on each buffer recieved from the RTAudio library.
int callback(void *output_buffer, void *input_buffer,
             unsigned int buffered_frames, double /* stream_time */,
             RtAudioStreamStatus status, void *data) {
  if (status) std::cout << "Stream over/underflow detected." << std::endl;

  auto *input = (SignalType *)input_buffer;
  auto *output = (SignalType *)output_buffer;
  auto *transform = (SignalTransformFn *)data;

  // There is 1 frame per each channel, so we actually need to double the number
  // of buffered frames when dealing with 2 channels.
  for (unsigned int frame = 0; frame < buffered_frames * 1; ++frame) {
    *(output + frame) = (*transform)(*(input + frame));
  }
  return 0;
}

class AudioTransformer {
 public:
  AudioTransformer(SignalTransformFn transform_fn)
      : transform_fn_(std::move(transform_fn)) {
    // Input from my guitar.
    // TODO(quinn): Read from stdin.
    input_stream_parameters_.deviceId = 3;
    input_stream_parameters_.nChannels = 1;

    // Output to the macbook speakers;
    // TODO(quinn): Read from stdin.
    output_stream_parameters_.deviceId = 2;
    output_stream_parameters_.nChannels = 1;

    unsigned int frames_to_buffer = 512;
    audio_interface_.openStream(&output_stream_parameters_,
                                &input_stream_parameters_, RTAUDIO_FLOAT32,
                                /* sample_rate = */ 44100, &frames_to_buffer,
                                &callback, &transform_fn_);
  }

  void Start() { audio_interface_.startStream(); }

 private:
  RtAudio audio_interface_;
  RtAudio::StreamParameters input_stream_parameters_;
  RtAudio::StreamParameters output_stream_parameters_;

  SignalTransformFn transform_fn_;
};

class PedalChain : public Pedal {
 public:
  using iterator = std::vector<std::unique_ptr<Pedal>>::iterator;

  SignalType Transform(SignalType signal) override {
    for (const auto &pedal : pedals_) {
      signal = pedal->Transform(signal);
    }
    // TODO(quinn): Might need to clamp this to prevent it from crackling.
    return signal;
  }

  std::string Describe() override {
    std::string result;
    for (auto &pedal : pedals_) {
      result += pedal->Describe() + "-->";
    }
    result += "amp";
    return result;
  }

  PedalChain &AddPedal(std::unique_ptr<Pedal> pedal) {
    pedals_.push_back(std::move(pedal));
    return *this;
  }

  iterator begin() { return pedals_.begin(); }
  iterator end() { return pedals_.end(); }

  void insert(iterator position, std::unique_ptr<Pedal> pedal) {
    pedals_.insert(position, std::move(pedal));
  }

  void remove(iterator position) { pedals_.erase(position); }

 private:
  std::vector<std::unique_ptr<Pedal>> pedals_;
};

int main() {
  probe();

  int count = 0;
  float min = 10000;
  float max = -1;

  std::mutex pedal_chain_mutex;
  PedalChain pedal_chain;
  // pedal_chain.AddPedal(std::unique_ptr<Pedal>(new BluesDriverPedal()));
  // pedal_chain.AddPedal(std::unique_ptr<Pedal>(new DelayPedal(16000)));
  // pedal_chain.AddPedal(std::unique_ptr<Pedal>(new DistortionPedal()));

  AudioTransformer at(
      [&count, &min, &max, &pedal_chain, &pedal_chain_mutex](SignalType input) {
        count = (count + 1) % 10000;
        if (count == 0) {
          // std::cout << "min signal: " << min << " max signal: " << max <<
          // "\r";
          // std::cout.flush();
        }

        SignalType val;
        {
          std::lock_guard<std::mutex> lock(pedal_chain_mutex);
          val = pedal_chain.Transform(input);
        }

        min = std::min(min, val);
        max = std::max(max, val);

        return val;
      });
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
        std::lock_guard<std::mutex> lock(pedal_chain_mutex);
        pedal_chain.insert(pedal_location, std::move(pedal));
        std::cout << pedal_chain.Describe() << std::endl;
      } else {
        std::cout << "Unknown pedal type: " << pedal_type << std::endl;
      }
    } else if (tokens[0] == "remove") {
      auto pedal_location = pedal_chain.begin() + std::stoi(tokens[1]);
      std::lock_guard<std::mutex> lock(pedal_chain_mutex);
      pedal_chain.remove(pedal_location);
      std::cout << pedal_chain.Describe() << std::endl;
    }
  }
  return 0;
}
