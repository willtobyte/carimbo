#pragma once

#include "common.hpp"

struct SDL_Deleter {
  void operator()(SDL_Surface *ptr) const noexcept {
    if (ptr) {
      SDL_DestroySurface(ptr);
    }
  }

  void operator()(SDL_Texture *ptr) const noexcept {
    if (ptr) {
      SDL_DestroyTexture(ptr);
    }
  }

  void operator()(SDL_Renderer *ptr) const noexcept {
    if (ptr) {
      SDL_DestroyRenderer(ptr);
    }
  }

  void operator()(SDL_Window *ptr) const noexcept {
    if (ptr) {
      SDL_DestroyWindow(ptr);
    }
  }

  void operator()(SDL_IOStream *ptr) const noexcept {
    if (ptr) {
      SDL_CloseIO(ptr);
    }
  }

  void operator()(SDL_Gamepad *ptr) const noexcept {
    if (ptr) {
      SDL_CloseGamepad(ptr);
    }
  }
};
