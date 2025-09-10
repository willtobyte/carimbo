#include "renderer.hpp"

#include "window.hpp"

using namespace graphics;

renderer::renderer(std::shared_ptr<window> window)
    : _window(std::move(window)) {
  constexpr const auto vsync =
    #ifdef SANDBOX
      0
    #else
      1
    #endif
    ;

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, *_window);
  SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, vsync);
  SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, nullptr);

  SDL_Renderer *renderer = SDL_CreateRendererWithProperties(props);
  if (!renderer) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_CreateRendererWithProperties] {}", SDL_GetError()));
  }

  // SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  _renderer.reset(renderer);
}

renderer::operator SDL_Renderer *() const noexcept {
  return _renderer.get();
}

renderer::operator SDL_Window *() const noexcept {
  return *_window;
}

void renderer::begin() const noexcept {
  SDL_RenderClear(*this);
}

void renderer::end() const noexcept {
  SDL_RenderPresent(*this);
}
