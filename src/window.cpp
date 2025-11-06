#include "window.hpp"

using namespace graphics;

window::window(const std::string& title, int32_t width, int32_t height, bool fullscreen)
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
  if (!_window) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_CreateWindow] {}", SDL_GetError()));
  }

  const SDL_Rect area = { 0, 0, width, height };
  const auto cursor = 0;
  SDL_SetTextInputArea(_window.get(), &area, cursor);
  SDL_StartTextInput(_window.get());
}

window::operator SDL_Window* () const {
  return _window.get();
}

std::shared_ptr<renderer> window::create_renderer(float scale) {
  const auto ptr = std::make_shared<renderer>(shared_from_this());

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
