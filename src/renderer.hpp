#pragma once

#include "common.hpp"

namespace graphics {
class renderer final {
public:
  explicit renderer(SDL_Window *window);
  ~renderer() noexcept = default;

  operator SDL_Renderer *() const noexcept;

  void begin() const noexcept;
  void end() const noexcept;

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
};
}
