#pragma once

#include "common.hpp"

class renderer final {
public:
  explicit renderer(std::shared_ptr<window> window);
  ~renderer() = default;

  operator SDL_Renderer* () const noexcept;

  operator SDL_Window* () const noexcept;

  void begin() const noexcept;
  void end() const noexcept;

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
  std::shared_ptr<window> _window;
};
