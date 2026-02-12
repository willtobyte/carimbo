#include "screen.hpp"

namespace screen {
  static float _width = 0.f;
  static float _height = 0.f;

  float width() noexcept { return _width; }
  float height() noexcept { return _height; }
  void present(float width, float height) noexcept { _width = width; _height = height; }
}
