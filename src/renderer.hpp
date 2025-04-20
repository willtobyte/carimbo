#pragma once

#include "common.hpp"

namespace graphics {
class renderer final {
public:
  explicit renderer(SDL_Window *window);
  ~renderer() = default;

  operator SDL_Renderer *();

  void begin();
  void end();

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
};
}
