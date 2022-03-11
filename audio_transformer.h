#ifndef AUDIO_TRANSFORMER_H
#define AUDIO_TRANSFORMER_H

#include <iostream>

#include "rtaudio/RtAudio.h"
#include "signal_type.h"

namespace internal {

// Settings required for reading and writing to the stream via the RtAudio
// callback.
struct StreamSettings {
  SignalTransformFn transform_fn;
  int num_output_channels;
};

SignalType clip(SignalType input) {
  if (input > 1)
    return 1;
  if (input < -1)
    return -1;
  return input;
}

// Callback on each buffer recieved from the RTAudio library. Applies the
// transformation function to the signal and writes the result into the output
// buffer.
int callback(void* output_buffer, void* input_buffer,
             unsigned int buffered_frames, double /* stream_time */,
             RtAudioStreamStatus status, void* data) {
  if (status) {
    std::cerr << "Stream over/underflow detected. Status: " << status
              << std::endl;
  }

  auto* input = (SignalType*)input_buffer;
  auto* output = (SignalType*)output_buffer;
  auto* stream_settings = (StreamSettings*)data;
  const auto& transform_fn = stream_settings->transform_fn;

  for (unsigned int frame = 0; frame < buffered_frames; ++frame) {
    auto transformed_input = clip(transform_fn(*input));
    ++input;
    for (int channel = 0; channel < stream_settings->num_output_channels;
         ++channel) {
      *output = transformed_input;
      ++output;
    }
  }
  return 0;
}

} // namespace internal

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
    const auto sample_rate = std::min(input_device.preferredSampleRate,
                                      output_device.preferredSampleRate);

    stream_settings_.transform_fn = std::move(transform_fn);
    stream_settings_.num_output_channels = output_device.outputChannels; // 1

    input_stream_parameters_.deviceId = input_device_index;
    // Most guitars just output mono.
    input_stream_parameters_.nChannels = 1;

    output_stream_parameters_.deviceId = output_device_index;
    output_stream_parameters_.nChannels = output_device.outputChannels; // 1

    audio_interface_.showWarnings(true);

    // 32 was choosen experimentally as the lowest possible value that doesn't
    // cause sound cards to complain.
    frames_to_buffer_ = 32;

    audio_interface_.openStream(
        &output_stream_parameters_, &input_stream_parameters_, RTAUDIO_FLOAT32,
        sample_rate, &frames_to_buffer_, &internal::callback, &stream_settings_,
        &stream_options_);
  }

  void Start() { audio_interface_.startStream(); }
  void Abort() { audio_interface_.abortStream(); }

  static void DumpDeviceInfo() {
    RtAudio audio_interface;
    unsigned int devices = audio_interface.getDeviceCount();
    for (unsigned int i = 0; i < devices; i++) {
      auto info = audio_interface.getDeviceInfo(i);
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
  RtAudio::StreamOptions stream_options_;
  internal::StreamSettings stream_settings_;
  // Zero means let RtAudio pick the smallest allowed buffer size. Smaller
  // buffers means lower latency as the library waits less for frames.
  unsigned int frames_to_buffer_ = 0;
};

#endif /* AUDIO_TRANSFORMER_H */
