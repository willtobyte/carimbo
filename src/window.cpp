#include "window.hpp"

using namespace graphics;

window::window(const std::string &title, int32_t width, int32_t height, bool fullscreen)
    : _width(width), _height(height),
      _window(
          SDL_CreateWindow(
              title.c_str(),
              width,
              height,
              fullscreen ? SDL_WINDOW_FULLSCREEN : 0
          ),
          SDL_Deleter{}
      ) {
  if (_window == nullptr) [[unlikely]] {
    panic("[SDL_CreateWindow] failed to create window: {}", SDL_GetError());
  }
}

window::operator SDL_Window *() {
  return _window.get();
}

std::shared_ptr<renderer> window::create_renderer(float_t scale) const {
  const auto ptr = std::make_shared<renderer>(_window.get());
  UNUSED(scale);
  SDL_SetRenderLogicalPresentation(*ptr, _width, _height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderScale(*ptr, scale, scale);

  return ptr;
}

int32_t window::width() const {
  return _width;
}

int32_t window::height() const {
  return _height;
}
