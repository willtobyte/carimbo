#include "point.hpp"

using namespace geometry;

point::point(float_t x, float_t y) noexcept
  : _x(x), _y(y) {}

void point::set(float_t x, float_t y) noexcept {
  _x = x;
  _y = y;
}

float_t point::x() const noexcept {
  return _x;
}

void point::set_x(float_t x) noexcept {
  _x = x;
}

float_t point::y() const noexcept {
  return _y;
}

void point::set_y(float_t y) noexcept {
  _y = y;
}

point::operator SDL_FPoint() const noexcept {
  return SDL_FPoint{_x, _y};
}

point point::operator+(const point& other) const noexcept {
  return point(_x + other._x, _y + other._y);
}

point& point::operator+=(const point& other) noexcept {
  _x += other._x;
  _y += other._y;

  return *this;
}

point& point::operator+=(const std::pair<uint8_t, float_t>& o) noexcept {
  static constexpr const auto X = static_cast<uint8_t>('x');
  static constexpr const auto Y = static_cast<uint8_t>('y');

  switch (o.first) {
    case X: _x += o.second; return *this;
    case Y: _y += o.second; return *this;
  }
  return *this;
}

point point::operator-(const size& rhs) const noexcept {
  return point(_x - rhs.width(), _y - rhs.height());
}

point point::operator-(const point& rhs) const noexcept {
  return point(_x - rhs._x, _y - rhs._y);
}
