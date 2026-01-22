#include "window.hpp"

window::window(std::string_view title, int width, int height, bool fullscreen)
    : _width(width), _height(height),
      _window(unwrap(
        std::unique_ptr<SDL_Window, SDL_Deleter>(
          SDL_CreateWindow(
            title.data(),
            width, height,
            fullscreen ? SDL_WINDOW_FULLSCREEN : 0
          )
        ),
        "error creating window"
      )) {
  const SDL_Rect area = { 0, 0, width, height };
  const auto cursor = 0;
  SDL_SetTextInputArea(_window.get(), &area, cursor);
  SDL_StartTextInput(_window.get());
}

window::operator SDL_Window* () const noexcept {
  return _window.get();
}

int window::width() const noexcept {
  return _width;
}

int window::height() const noexcept {
  return _height;
}
