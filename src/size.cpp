#include "size.hpp"

using namespace geometry;

size::size()
    : _width(0), _height(0) {}

size::size(float width, float height)
    : _width(width), _height(height) {}

void size::set_width(float width) {
  _width = width;
}

float size::width() const {
  return _width;
}

void size::set_height(float height) {
  _height = height;
}

float size::height() const {
  return _height;
}

bool size::operator==(const size& rhs) const {
  return std::fabs(_width - rhs._width) <= epsilon * std::max(std::fabs(_width), std::fabs(rhs._width))
    && std::fabs(_height - rhs._height) <= epsilon * std::max(std::fabs(_height), std::fabs(rhs._height));
}

bool size::operator!=(const size& rhs) const {
  return !(*this == rhs);
}

size size::operator*(float factor) const {
  return {
      _width * factor,
      _height * factor
  };
}

size size::operator/(float factor) const {
  return {
      _width / factor,
      _height / factor
  };
}
