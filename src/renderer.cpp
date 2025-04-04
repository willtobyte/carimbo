#include "renderer.hpp"

using namespace graphics;

renderer::renderer(SDL_Window *window)
    : _renderer(SDL_CreateRenderer(window, nullptr), SDL_Deleter{}) {
  if (!_renderer) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateRenderer] failed to create renderer: {}", SDL_GetError()));
  }
}

renderer::operator SDL_Renderer *() noexcept {
  return _renderer.get();
}

void renderer::begin() noexcept {
  SDL_RenderClear(*this);
}

void renderer::end() noexcept {
  SDL_RenderPresent(*this);
}
