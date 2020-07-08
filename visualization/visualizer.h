#ifndef VISUALIZER_H
#define VISUALIZER_H

#include "visualization/frame_buffer.h"
#include "visualization/screen.h"

#include <SDL2/SDL.h>
#include <chrono>
#include <iostream>
#include <thread>

class Visualizer {
 public:
  Visualizer(FrameBuffer* frame_buffer, int fps)
      : frame_buffer_(frame_buffer),
        screen_(/* title= */ "Audio Signal Visualizer", /* x= */ 100,
                /* y= */ 100, /* width= */ 500, /* height= */ 500),
        seconds_per_frame_(1.0 / fps) {}

  // Start the UI process and block the current thread until the window is
  // closed.
  void BlockingStart() {
    while (!CheckQuit()) {
      using Clock = std::chrono::high_resolution_clock;
      auto frame_start = Clock::now();
      RenderSignal();
      auto frame_duration = Clock::now() - frame_start;
      if (frame_duration < seconds_per_frame_) {
        std::this_thread::sleep_for(seconds_per_frame_ - frame_duration);
      } else {
        std::cout << "frame was too slow" << std::endl;
      }
    }
    screen_.Close();
  }

 private:
  // Render the audio signal from the frame buffer to the screen.
  void RenderSignal() {
    std::vector<SignalType> frames;
    frame_buffer_->Consume(&frames);

    // Change the x-coordinate so that the entire screen is spanned after each
    // point.
    int frame_index = 0;
    double dx = 500.0 / frames.size();

    std::vector<SDL_Point> points;
    for (SignalType signal : frames) {
      int x = dx * frame_index;
      // Place the origin in the middle of the screen and scale the height by
      // the audio signal.
      int y = (250 * signal) + 250;

      points.push_back({x, y});
      ++frame_index;
    }

    screen_.Clear();
    screen_.DrawLines(points);
    screen_.Update();
  }

  bool CheckQuit() {
    SDL_Event sdl_event;
    bool got_event = SDL_PollEvent(&sdl_event);

    if (!got_event) {
      return false;
    }

    return sdl_event.type == SDL_QUIT;
  }

  FrameBuffer* frame_buffer_;
  Screen screen_;
  std::chrono::duration<double> seconds_per_frame_;
};

#endif /* VISUALIZER_H */
