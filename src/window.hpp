#pragma once

#include "common.hpp"

namespace graphics {
class window final : public std::enable_shared_from_this<window> {
public:
  window(std::string_view title, int width, int height, bool fullscreen);
  ~window() = default;

  operator SDL_Window*() const;

  std::shared_ptr<renderer> create_renderer(float scale);

  int width() const;

  int height() const;

private:
  int _width;
  int _height;
  std::unique_ptr<SDL_Window, SDL_Deleter> _window;
};
}
