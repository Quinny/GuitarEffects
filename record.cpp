#include <fstream>

#include "audio_transformer.h"
#include "rtaudio/RtAudio.h"
#include "signal_type.h"

// Record audio signal and dumps it to a file called "recording" which can then
// be read by a Playback object (see playback.h).
int main() {
  std::vector<SignalType> recorded_signal;
  AudioTransformer at(
      [&recorded_signal](SignalType input) {
        recorded_signal.push_back(input);
        return input;
      },
      /* input_device_index= */ 3, /* output_device_index= */ 1,
      /* treat_input_as_mono= */ true);

  at.Start();
  std::cout << "Press any key to stop recording" << std::endl;
  char block;
  std::cin >> block;
  at.Abort();

  std::cout << "Recorded " << recorded_signal.size() << " frames" << std::endl;

  std::ofstream output_file("recording");
  output_file << recorded_signal.size() << " ";
  for (const auto& signal : recorded_signal) {
    output_file << signal << " ";
  }
  output_file.close();

  return 0;
}
