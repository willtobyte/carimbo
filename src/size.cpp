#include "size.hpp"

using namespace geometry;

size::size() noexcept
    : _width(0), _height(0) {}

size::size(float width, float height) noexcept
    : _width(width), _height(height) {}

void size::set_width(float width) noexcept {
  _width = width;
}

float size::width() const noexcept {
  return _width;
}

void size::set_height(float height) noexcept {
  _height = height;
}

float size::height() const noexcept {
  return _height;
}

bool size::operator==(const size& rhs) const noexcept {
  constexpr float epsilon = std::numeric_limits<float>::epsilon();
  return std::abs(_width - rhs._width) < epsilon && std::abs(_height - rhs._height) < epsilon;
}

bool size::operator!=(const size& rhs) const noexcept {
  return !(*this == rhs);
}

size size::operator*(float factor) const noexcept {
  return {
      _width * factor,
      _height * factor
  };
}

size size::operator/(float factor) const noexcept {
  return {
      _width / factor,
      _height / factor
  };
}
