#include "renderer.hpp"

#include "defer.hpp"
#include "window.hpp"

renderer::renderer(std::shared_ptr<window> window)
    : _window(std::move(window)) {
  const auto vsync =
    std::getenv("NOVSYNC") ? 0 : 1;

  const auto props = SDL_CreateProperties();
  defer(SDL_DestroyProperties(props));

  SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, *_window);
  SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, vsync);
  SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, nullptr);

  _renderer = std::unique_ptr<SDL_Renderer, SDL_Deleter>(SDL_CreateRendererWithProperties(props));
}

renderer::operator SDL_Renderer* () const noexcept {
  return _renderer.get();
}

renderer::operator SDL_Window* () const noexcept {
  return *_window;
}

void renderer::begin() const noexcept {
  SDL_RenderClear(*this);
}

void renderer::end() const noexcept {
  SDL_RenderPresent(*this);
}
