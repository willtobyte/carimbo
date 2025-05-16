#include "framerate.hpp"

using namespace framework;

void framerate::loop(float_t delta) {
  UNUSED(delta);
  _frames++;

  const auto now = SDL_GetTicks();
  _elapsed += now - _start;
  _start = now;

  if (_elapsed >= 1000) {
    fmt::println("{:.1f}", static_cast<double>(_frames * _elapsed) * 0.001);;

    _elapsed = 0;
    _frames = 0;
  }
}
