#pragma once

#include "common.hpp"

class window final {
public:
  window(std::string_view title, int width, int height, bool fullscreen);
  ~window() = default;

  operator SDL_Window*() const noexcept;

  int width() const noexcept;

  int height() const noexcept;

private:
  int _width;
  int _height;
  std::unique_ptr<SDL_Window, SDL_Deleter> _window;
};
