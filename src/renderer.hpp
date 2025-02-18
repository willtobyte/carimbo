#pragma once

#include "common.hpp"

namespace graphics {
class renderer {
public:
  explicit renderer(SDL_Window *window);
  ~renderer() noexcept = default;

  operator SDL_Renderer *() noexcept;

  void begin() noexcept;
  void end() noexcept;

  void draw(std::span<const uint32_t> pixels);

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
};
}
