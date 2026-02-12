#include "screen.hpp"

namespace screen {
  static float _width{.0f};
  static float _height{.0f};

  float width() noexcept { return _width; }
  float height() noexcept { return _height; }
  void present(float width, float height) noexcept { _width = width; _height = height; }
}
