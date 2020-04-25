#ifndef AUDIO_TRANSFORMER_H
#define AUDIO_TRANSFORMER_H

#include "rtaudio/RtAudio.h"
#include "signal.h"

namespace internal {

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
  //
  // TODO: The number of channels here should be variable.
  for (unsigned int frame = 0; frame < buffered_frames * 2; ++frame) {
    *(output + frame) = (*transform)(*(input + frame));
  }
  return 0;
}

}  // namespace internal

class AudioTransformer {
 public:
  AudioTransformer(SignalTransformFn transform_fn, int input_device_index,
                   int output_device_index)
      : transform_fn_(std::move(transform_fn)) {
    const auto input_device =
        audio_interface_.getDeviceInfo(input_device_index);
    const auto output_device =
        audio_interface_.getDeviceInfo(output_device_index);

    const auto num_channels =
        std::min(input_device.inputChannels, output_device.outputChannels);

    input_stream_parameters_.deviceId = 3;
    input_stream_parameters_.nChannels = num_channels;

    output_stream_parameters_.deviceId = 1;
    output_stream_parameters_.nChannels = num_channels;

    audio_interface_.showWarnings(true);

    // TODO: What is the optimal value here.
    unsigned int frames_to_buffer = 256;

    auto sample_rate = std::min(input_device.preferredSampleRate,
                                output_device.preferredSampleRate);
    audio_interface_.openStream(
        &output_stream_parameters_, &input_stream_parameters_, RTAUDIO_FLOAT32,
        sample_rate, &frames_to_buffer, &internal::callback, &transform_fn_);
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

  SignalTransformFn transform_fn_;
};

#endif /* AUDIO_TRANSFORMER_H */
