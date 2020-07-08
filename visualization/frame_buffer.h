#ifndef FRAME_BUFFER_H
#define FRAME_BUFFER_H

#include "signal_type.h"

#include <mutex>
#include <vector>

// A thread safe buffer for collecting and consuming audio samples.
class FrameBuffer {
 public:
  // Once the buffer exceeds the max size, new samples will be rejected. This
  // is to limit the buffers memory usage in the event that the consumer lags.
  FrameBuffer(int max_size) : max_size_(max_size) {}

  // Consume the contents of the buffer into the destination. After this, the
  // buffer will be empty.
  void Consume(std::vector<SignalType>* dst) {
    std::lock_guard<std::mutex> lock(mu_);
    *dst = std::move(buffer_);
    buffer_.clear();
  }

  // Add a signal to the buffer.
  void Add(SignalType signal) {
    std::lock_guard<std::mutex> lock(mu_);
    if (buffer_.size() >= max_size_) return;
    buffer_.push_back(signal);
  }

 private:
  std::vector<SignalType> buffer_;
  int max_size_;
  std::mutex mu_;
};

#endif /* FRAME_BUFFER_H */
