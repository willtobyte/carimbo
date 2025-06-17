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

point point::operator+(const point &other) const noexcept {
  return point(_x + other._x, _y + other._y);
}

point &point::operator+=(const point &other) noexcept {
  _x += other._x;
  _y += other._y;

  return *this;
}

point &point::operator+=(std::pair<uint8_t, float_t> offset) noexcept{
  if (offset.first == 'x') {
    _x += offset.second;
  } else if (offset.first == 'y') {
    _y += offset.second;
  }

  return *this;
}

point point::operator-(const size &rhs) const noexcept {
  return point(_x - rhs.width(), _y - rhs.height());
}

point point::operator-(const point &rhs) const noexcept {
  return point(_x - rhs._x, _y - rhs._y);
}
