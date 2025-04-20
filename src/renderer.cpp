#include "renderer.hpp"

using namespace graphics;

renderer::renderer(SDL_Window *window)
    : _renderer(SDL_CreateRenderer(window, nullptr), SDL_Deleter{}) {
  if (!_renderer) [[unlikely]] {
    panic("[SDL_CreateRenderer] failed to create renderer: {}", SDL_GetError());
  }
}

renderer::operator SDL_Renderer *() {
  return _renderer.get();
}

void renderer::begin() {
  SDL_RenderClear(*this);
}

void renderer::end() {
  SDL_RenderPresent(*this);
}
