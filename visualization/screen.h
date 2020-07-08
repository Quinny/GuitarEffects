#ifndef SCREEN_H
#define SCREEN_H

#include <SDL2/SDL.h>

#include "visualization/sdl-ptrs.h"

// A nice wrapper around an SDL screen.
class Screen {
 public:
  Screen(const std::string& title, const int x, const int y, const int w,
         const int h) {
    SDL_Init(SDL_INIT_VIDEO);
    window_.reset(
        SDL_CreateWindow(title.c_str(), x, y, w, h, SDL_WINDOW_SHOWN));
    window_surface_.reset(SDL_GetWindowSurface(window_.get()));
    renderer_.reset(SDL_GetRenderer(window_.get()));
  }

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
};

#endif /* SCREEN_H */
