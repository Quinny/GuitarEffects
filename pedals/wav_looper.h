#ifndef WAV_LOOPER_H
#define WAV_LOOPER_H

#include "AudioFile/AudioFile.h"
#include "pedal_registry.h"

#include <dirent.h>

// A pedal which allows for looping of WAV files ontop of the input signal.
class WavLooper : public Pedal {
 public:
  // The provided directory is where the pedal will look for files to load.
  WavLooper(const std::string& directory)
      : file_paths_(GetAllFiles(directory)) {
    current_file_.load(file_paths_[current_file_index_]);
  }

  SignalType Transform(SignalType input) override {
    // Blend the input and the wav file frame together.
    auto output = input + (wav_blend_ *
                           current_file_.samples[channel_index_][frame_index_]);

    // TODO: This will only work with 2 channel wav file input.
    channel_index_++;
    if (channel_index_ >= current_file_.getNumChannels()) {
      channel_index_ = 0;
      frame_index_ =
          (frame_index_ + 1) % current_file_.getNumSamplesPerChannel();
    }
    return output;
  }

  PedalInfo Describe() override {
    PedalInfo info;
    info.name = "WavLooper";

    info.knobs = {
        PedalKnob{.name = "file",
                  .value = static_cast<double>(current_file_index_),
                  .tweak_amount = 1},
        PedalKnob{
            .name = "wav_blend", .value = wav_blend_, .tweak_amount = 0.1},
    };

    return info;
  }

  void AdjustKnob(const PedalKnob& pedal_knob) override {
    if (pedal_knob.name == "file") {
      current_file_index_ = static_cast<int>(pedal_knob.value);
      // Ensure that we don't go negative and that we loop around when we
      // surpass the number of files.
      current_file_index_ =
          std::max(current_file_index_, 0) % file_paths_.size();
      current_file_.load(file_paths_[current_file_index_]);
    } else if (pedal_knob.name == "wav_blend") {
      wav_blend_ = pedal_knob.value;
    }
  }

 private:
  // TODO: Is there a more graceful way to handle failures than just asserting
  // here?
  std::vector<std::string> GetAllFiles(const std::string& directory_path) {
    DIR* directory = opendir(directory_path.c_str());
    assert(directory != nullptr);

    std::vector<std::string> files;
    struct dirent* entity;
    for (struct dirent* entity = readdir(directory); entity != nullptr;
         entity = readdir(directory)) {
      auto entity_name = directory_path + std::string(entity->d_name);
      if (entity->d_type == DT_REG) {
        files.push_back(std::move(entity_name));
      }
    }

    // The files can't be empty otherwise this will segfault.
    assert(!files.empty());

    closedir(directory);
    return files;
  }

  int channel_index_ = 0;
  int frame_index_ = 0;
  int current_file_index_ = 0;
  double wav_blend_ = 1.0;
  AudioFile<SignalType> current_file_;
  std::vector<std::string> file_paths_;
};

REGISTER_PEDAL("WavLooper", []() {
  return std::unique_ptr<Pedal>(new WavLooper("../pedals/wav_files/"));
});

#endif /* WAV_LOOPER_H */
