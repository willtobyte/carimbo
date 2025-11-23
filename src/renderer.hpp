#pragma once

#include "common.hpp"

class window;

class renderer final {
public:
  explicit renderer(std::shared_ptr<window> window);
  ~renderer() = default;

  operator SDL_Renderer* () const;

  operator SDL_Window* () const;

  void begin() const;
  void end() const;

private:
  std::unique_ptr<SDL_Renderer, SDL_Deleter> _renderer;
  std::shared_ptr<window> _window;
};
