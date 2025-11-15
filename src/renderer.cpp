#include "renderer.hpp"

#include "defer.hpp"
#include "window.hpp"

using namespace graphics;

renderer::renderer(std::shared_ptr<window> window)
    : _window(std::move(window)) {
  const auto vsync =
    std::getenv("NOVSYNC") ? 0 : 1;

  const auto props = SDL_CreateProperties();
  defer(SDL_DestroyProperties(props));

  SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, *_window);
  SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, vsync);
  SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, nullptr);

  auto* renderer = SDL_CreateRendererWithProperties(props);
  if (!renderer) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_CreateRendererWithProperties] {}", SDL_GetError()));
  }

  _renderer.reset(renderer);
}

renderer::operator SDL_Renderer* () const {
  return _renderer.get();
}

renderer::operator SDL_Window* () const {
  return *_window;
}

void renderer::begin() const {
  SDL_RenderClear(*this);
}

void renderer::end() const {
  SDL_RenderPresent(*this);
}
