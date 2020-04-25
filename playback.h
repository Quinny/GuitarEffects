#ifndef PLAYBACK_H
#define PLAYBACK_H

#include <fstream>
#include <vector>

#include "signal.h"

// Reads a recording file (see record.cpp to generate such files) into memory
// and replays the signal in a loop through the "next()" method.
//
// This is useful when debugging/testing new pedals as you don't need to keep
// your guitar around. You can just make a recording and then loop it back
// through the effects loop.
class Playback {
 public:
  Playback(const std::string& filename) {
    std::ifstream stream(filename);
    SignalType signal;
    while (stream >> signal) {
      frames_.push_back(signal);
    }
  }

  SignalType next() {
    auto signal = frames_[next_frame_];
    next_frame_ = (next_frame_ + 1) % frames_.size();
    return signal;
  }

 private:
  std::vector<SignalType> frames_;
  int next_frame_ = 0;
};

#endif /* PLAYBACK_H */
