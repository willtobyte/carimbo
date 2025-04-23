#include "framerate.hpp"

using namespace framework;

void framerate::loop(float_t delta) {
  UNUSED(delta);
  _frames++;

  const auto now = SDL_GetTicks();
  _elapsed += now - _start;
  _start = now;

  if (_elapsed >= 1000) {
    fmt::println("{:.1f}", static_cast<float>(_frames) * 0.001f * static_cast<float>(_elapsed));

    _elapsed = 0;
    _frames = 0;
  }
}
