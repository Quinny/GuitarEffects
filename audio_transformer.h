#ifndef AUDIO_TRANSFORMER_H
#define AUDIO_TRANSFORMER_H

#include "rtaudio/RtAudio.h"
#include "signal.h"

namespace internal {

struct StreamSettings {
  SignalTransformFn transform_fn;
  int num_channels;
};

// Callback on each buffer recieved from the RTAudio library. Applies the
// transformation function to the signal and writes the result into the output
// buffer.
int callback(void *output_buffer, void *input_buffer,
             unsigned int buffered_frames, double /* stream_time */,
             RtAudioStreamStatus status, void *data) {
  if (status) std::cout << "Stream over/underflow detected." << std::endl;

  auto *input = (SignalType *)input_buffer;
  auto *output = (SignalType *)output_buffer;
  auto *stream_settings = (StreamSettings *)data;
  const auto &transform_fn = stream_settings->transform_fn;

  // There is 1 frame per each channel, so we need to read signal from each
  // channel.
  for (unsigned int frame = 0;
       frame < buffered_frames * stream_settings->num_channels; ++frame) {
    *(output + frame) = transform_fn(*(input + frame));
  }
  return 0;
}

}  // namespace internal

class AudioTransformer {
 public:
  // Open an audio stream that reads signal from `input_device_index`,
  // transforms it using `transform_fn` and writes the result to
  // `output_device_index`.
  AudioTransformer(SignalTransformFn transform_fn, int input_device_index,
                   int output_device_index) {
    const auto input_device =
        audio_interface_.getDeviceInfo(input_device_index);
    const auto output_device =
        audio_interface_.getDeviceInfo(output_device_index);

    const auto num_channels =
        std::min(input_device.inputChannels, output_device.outputChannels);

    stream_settings_.transform_fn = std::move(transform_fn);
    stream_settings_.num_channels = num_channels;

    input_stream_parameters_.deviceId = input_device_index;
    input_stream_parameters_.nChannels = num_channels;

    output_stream_parameters_.deviceId = output_device_index;
    output_stream_parameters_.nChannels = num_channels;

    audio_interface_.showWarnings(true);

    // TODO: What is the optimal value here?
    unsigned int frames_to_buffer = 256;

    auto sample_rate = std::min(input_device.preferredSampleRate,
                                output_device.preferredSampleRate);
    audio_interface_.openStream(
        &output_stream_parameters_, &input_stream_parameters_, RTAUDIO_FLOAT32,
        sample_rate, &frames_to_buffer, &internal::callback, &stream_settings_);
  }

  void Start() { audio_interface_.startStream(); }
  void Abort() { audio_interface_.abortStream(); }

  void DumpDeviceInfo() {
    unsigned int devices = audio_interface_.getDeviceCount();
    for (unsigned int i = 0; i < devices; i++) {
      auto info = audio_interface_.getDeviceInfo(i);
      if (info.probed == true) {
        std::cout << "device ID = " << i << std::endl;
        std::cout << "device name: " << info.name << std::endl;
        std::cout << ": maximum output channels = " << info.outputChannels
                  << std::endl;
        std::cout << ": maximum input channels = " << info.inputChannels
                  << std::endl;
        std::cout << "Native format: " << info.nativeFormats << std::endl;
        std::cout << "Sample rate " << info.preferredSampleRate << std::endl;
      }
    }
  }

 private:
  RtAudio audio_interface_;
  RtAudio::StreamParameters input_stream_parameters_;
  RtAudio::StreamParameters output_stream_parameters_;
  internal::StreamSettings stream_settings_;
};

#endif /* AUDIO_TRANSFORMER_H */
