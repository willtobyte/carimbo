#pragma once

#include "common.hpp"

namespace graphics {
class window;

class renderer final {
public:
  explicit renderer(std::shared_ptr<window> window);
  ~renderer() noexcept = default;

  operator SDL_Renderer* () const noexcept;

  operator SDL_Window* () const noexcept;

  void begin() const noexcept;
  void end() const noexcept;

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
  std::shared_ptr<window> _window;
};
}
