#include "vector2d.hpp"

using namespace algebra;

vector2d::vector2d() noexcept
    : _x(0), _y(0) {}

vector2d::vector2d(float_t x, float_t y) noexcept
    : _x(x), _y(y) {}

float_t vector2d::x() const noexcept {
  return _x;
}

float_t vector2d::y() const noexcept {
  return _y;
}

void vector2d::set_x(float_t x) noexcept {
  _x = x;
}

void vector2d::set_y(float_t y) noexcept {
  _y = y;
}

void vector2d::set(float_t x, float_t y) noexcept {
  _x = x;
  _y = y;
}

vector2d vector2d::operator+(const vector2d &other) const noexcept {
  return vector2d(_x + other._x, _y + other._y);
}

vector2d vector2d::operator-(const vector2d &other) const noexcept {
  return vector2d(_x - other._x, _y - other._y);
}

vector2d vector2d::operator*(float_t scalar) const noexcept {
  return vector2d(_x * scalar, _y * scalar);
}

vector2d vector2d::operator/(float_t scalar) const noexcept {
  return vector2d(_x / scalar, _y / scalar);
}

vector2d &vector2d::operator+=(const vector2d &other) noexcept {
  _x += other._x;
  _y += other._y;
  return *this;
}

vector2d &vector2d::operator-=(const vector2d &other) noexcept {
  _x -= other._x;
  _y -= other._y;
  return *this;
}

vector2d &vector2d::operator*=(float_t scalar) noexcept {
  _x *= scalar;
  _y *= scalar;
  return *this;
}

vector2d &vector2d::operator/=(float_t scalar) noexcept {
  _x /= scalar;
  _y /= scalar;
  return *this;
}

bool vector2d::operator==(const vector2d &other) const noexcept {
  return _x == other._x && _y == other._y;
}

bool vector2d::operator!=(const vector2d &other) const noexcept {
  return !(*this == other);
}

float_t vector2d::magnitude() const noexcept {
  return std::sqrt(_x * _x + _y * _y);
}

vector2d vector2d::unit() const noexcept {
  const auto m = magnitude();
  return (m < std::numeric_limits<float_t>::epsilon()) ? vector2d(0, 0) : *this / m;
}

float_t vector2d::dot(const vector2d &other) const noexcept {
  return _x * other._x + _y * other._y;
}

bool vector2d::moving() const noexcept {
  return !zero();
}

bool vector2d::right() const noexcept {
  return _x > 0;
}

bool vector2d::left() const noexcept {
  return _x < 0;
}

bool vector2d::zero() const noexcept {
  return _x == 0 && _y == 0;
}
