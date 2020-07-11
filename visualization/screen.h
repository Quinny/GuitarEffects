#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>

#include "visualization/sdl-ptrs.h"

// A nice wrapper around an SDL screen.
class Screen {
 public:
  Screen(const std::string& title) {
    SDL_Init(SDL_INIT_VIDEO);
    SDL_DisplayMode display_mode;
    SDL_GetCurrentDisplayMode(/* display_index = */ 0, &display_mode);
    width_ = display_mode.w;
    height_ = display_mode.h;

    window_.reset(SDL_CreateWindow(title.c_str(), /* x= */ 0, /* y= */ 0,
                                   width_, height_, SDL_WINDOW_SHOWN));
    window_surface_.reset(SDL_GetWindowSurface(window_.get()));
    renderer_.reset(SDL_GetRenderer(window_.get()));
  }

  int width() { return width_; }
  int height() { return height_; }

  void Update() { SDL_RenderPresent(renderer_.get()); }

  void DrawLines(const std::vector<SDL_Point>& points) {
    SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);
    SDL_RenderDrawLines(renderer_.get(), points.data(), points.size());
  }

  void Clear() {
    SDL_SetRenderDrawColor(renderer_.get(), 0, 0, 0, 255);
    SDL_RenderClear(renderer_.get());
  }

  void Close() {
    if (!open_) {
      return;
    }

    window_surface_.reset();
    window_.reset();
    renderer_.reset();
    SDL_Quit();
    open_ = false;
  }

  ~Screen() { Close(); }

 private:
  SdlWindowPtr window_;
  SdlSurfacePtr window_surface_;
  SdlRendererPtr renderer_;
  bool open_ = true;
  int width_;
  int height_;
};

#endif /* SCREEN_H */
