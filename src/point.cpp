#include "point.hpp"

using namespace geometry;

point::point(float x, float y) noexcept
  : _x(x), _y(y) {}

void point::set(float x, float y) noexcept {
  _x = x;
  _y = y;
}

float point::x() const noexcept {
  return _x;
}

void point::set_x(float x) noexcept {
  _x = x;
}

float point::y() const noexcept {
  return _y;
}

void point::set_y(float y) noexcept {
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

point& point::operator+=(const std::pair<char, float>& o) noexcept {
  switch (o.first) {
    case 'x': _x += o.second; return *this;
    case 'y': _y += o.second; return *this;
  }

  return *this;
}

point point::operator-(const size& rhs) const noexcept {
  return point(_x - rhs.width(), _y - rhs.height());
}

point point::operator-(const point& rhs) const noexcept {
  return point(_x - rhs._x, _y - rhs._y);
}
