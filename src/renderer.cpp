#include "renderer.hpp"

using namespace graphics;

renderer::renderer(SDL_Window *window) {
  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetPointerProperty(props, SDL_PROP_RENDERER_CREATE_WINDOW_POINTER, window);
  SDL_SetNumberProperty(props, SDL_PROP_RENDERER_CREATE_PRESENT_VSYNC_NUMBER, 1);
  SDL_SetStringProperty(props, SDL_PROP_RENDERER_CREATE_NAME_STRING, nullptr);


  SDL_Renderer *renderer = SDL_CreateRendererWithProperties(props);
  if (!renderer) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateRendererWithProperties] {}", SDL_GetError()));
  }

  _renderer.reset(renderer);
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
