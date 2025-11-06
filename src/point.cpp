#include "point.hpp"

using namespace geometry;

point::point(float x, float y)
  : _x(x), _y(y) {}

void point::set(float x, float y) {
  _x = x;
  _y = y;
}

float point::x() const {
  return _x;
}

void point::set_x(float x) {
  _x = x;
}

float point::y() const {
  return _y;
}

void point::set_y(float y) {
  _y = y;
}

point::operator SDL_FPoint() const {
  return SDL_FPoint{_x, _y};
}

point point::operator+(const point& other) const {
  return point(_x + other._x, _y + other._y);
}

point& point::operator+=(const point& other) {
  _x += other._x;
  _y += other._y;

  return *this;
}

point& point::operator+=(const std::pair<char, float>& o) {
  switch (o.first) {
    case 'x': _x += o.second; return *this;
    case 'y': _y += o.second; return *this;
  }

  return *this;
}

point point::operator-(const size& rhs) const {
  return point(_x - rhs.width(), _y - rhs.height());
}

point point::operator-(const point& rhs) const {
  return point(_x - rhs._x, _y - rhs._y);
}

bool point::operator==(const point& other) const {
  return std::fabs(_x - other._x) <= epsilon * std::max(std::fabs(_x), std::fabs(other._x))
    && std::fabs(_y - other._y) <= epsilon * std::max(std::fabs(_y), std::fabs(other._y));
}

bool point::operator!=(const point& other) const {
  return !(*this == other);
}
