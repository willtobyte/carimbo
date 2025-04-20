#include "point.hpp"

using namespace geometry;

point::point(float_t x, float_t y)
    : _x(x), _y(y) {}

void point::set(float_t x, float_t y) {
  _x = x;
  _y = y;
}

float_t point::x() const {
  return _x;
}

void point::set_x(float_t x) {
  _x = x;
}

float_t point::y() const {
  return _y;
}

void point::set_y(float_t y) {
  _y = y;
}

point::operator SDL_FPoint() const {
  return SDL_FPoint{_x, _y};
}

point point::operator+(const point &other) const {
  return point(_x + other._x, _y + other._y);
}

point &point::operator+=(const point &other) {
  _x += other._x;
  _y += other._y;

  return *this;
}

point &point::operator+=(std::pair<char, float_t> offset) {
  if (offset.first == 'x') {
    _x += offset.second;
  } else if (offset.first == 'y') {
    _y += offset.second;
  }

  return *this;
}

point point::operator-(const size &rhs) const {
  return point(_x - rhs.width(), _y - rhs.height());
}

point point::operator-(const point &rhs) const {
  return point(_x - rhs._x, _y - rhs._y);
}
