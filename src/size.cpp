#include "size.hpp"

using namespace geometry;

size::size()
    : _width(0), _height(0) {}

size::size(float_t width, float_t height)
    : _width(width), _height(height) {}

void size::set_width(float_t width) {
  _width = width;
}

float_t size::width() const {
  return _width;
}

void size::set_height(float_t height) {
  _height = height;
}

float_t size::height() const {
  return _height;
}

bool size::operator==(const size &rhs) const {
  return _width == rhs._width && _height == rhs._height;
}

bool size::operator!=(const size &rhs) const {
  return !(*this == rhs);
}

size size::operator*(float_t factor) const {
  return {
      _width * factor,
      _height * factor
  };
}

size size::operator/(float_t factor) const {
  assert(factor != 0);
  return {
      _width / factor,
      _height / factor
  };
}
