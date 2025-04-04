#include "size.hpp"

using namespace geometry;

size::size() noexcept
    : _width(0), _height(0) {}

size::size(float_t width, float_t height) noexcept
    : _width(width), _height(height) {}

void size::set_width(float_t width) noexcept {
  _width = width;
}

float_t size::width() const noexcept {
  return _width;
}

void size::set_height(float_t height) noexcept {
  _height = height;
}

float_t size::height() const noexcept {
  return _height;
}

bool size::operator==(const size &rhs) const noexcept {
  return _width == rhs._width && _height == rhs._height;
}

bool size::operator!=(const size &rhs) const noexcept {
  return !(*this == rhs);
}

size size::operator*(float_t factor) const noexcept {
  return {
      _width * factor,
      _height * factor
  };
}
