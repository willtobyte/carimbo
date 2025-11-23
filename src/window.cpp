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

window::operator SDL_Window* () const {
  return _window.get();
}

std::shared_ptr<renderer> window::create_renderer(float scale) {
  const auto ptr = std::make_shared<renderer>(shared_from_this());

  SDL_SetRenderLogicalPresentation(*ptr, _width, _height, SDL_LOGICAL_PRESENTATION_LETTERBOX);
  SDL_SetRenderScale(*ptr, scale, scale);

  return ptr;
}

int window::width() const {
  return _width;
}

int window::height() const {
  return _height;
}
