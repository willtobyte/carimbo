#include "point.hpp"

using namespace geometry;

point::point(int32_t x, int32_t y) noexcept
    : _x(x), _y(y) {}

void point::set(int32_t x, int32_t y) noexcept {
  _x = x;
  _y = y;
}

int32_t point::x() const noexcept {
  return _x;
}

void point::set_x(int32_t x) noexcept {
  _x = x;
}

int32_t point::y() const noexcept {
  return _y;
}

void point::set_y(int32_t y) noexcept {
  _y = y;
}

point::operator SDL_Point() const noexcept {
  return SDL_Point{_x, _y};
}

point point::operator+(const point &other) const noexcept {
  return point(_x + other._x, _y + other._y);
}

point &point::operator+=(const point &other) noexcept {
  _x += other._x;
  _y += other._y;
  return *this;
}

point &point::operator+=(std::pair<char, int32_t> offset) noexcept {
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
