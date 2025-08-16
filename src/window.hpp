#pragma once

#include "common.hpp"

namespace graphics {
class window final : public std::enable_shared_from_this<window> {
public:
  window(const std::string& title, int32_t width, int32_t height, bool fullscreen);
  ~window() = default;

  operator SDL_Window *() const noexcept;

  std::shared_ptr<renderer> create_renderer(float_t scale);

  int32_t width() const noexcept;

  int32_t height() const noexcept;

private:
  int32_t _width;
  int32_t _height;
  std::unique_ptr<SDL_Window, SDL_Deleter> _window;
};
}
