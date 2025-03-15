#include "vector2d.hpp"

using namespace algebra;

vector2d::vector2d() noexcept
    : _x(0), _y(0) {}

vector2d::vector2d(int32_t x, int32_t y) noexcept
    : _x(x), _y(y) {}

int32_t vector2d::x() const noexcept {
  return _x;
}

int32_t vector2d::y() const noexcept {
  return _y;
}

void vector2d::set_x(int32_t x) noexcept {
  _x = x;
}

void vector2d::set_y(int32_t y) noexcept {
  _y = y;
}

void vector2d::set(int32_t x, int32_t y) noexcept {
  _x = x;
  _y = y;
}

vector2d vector2d::operator+(const vector2d &other) const noexcept {
  return vector2d(_x + other._x, _y + other._y);
}

vector2d vector2d::operator-(const vector2d &other) const noexcept {
  return vector2d(_x - other._x, _y - other._y);
}

vector2d vector2d::operator*(int32_t scalar) const noexcept {
  return vector2d(_x * scalar, _y * scalar);
}

vector2d vector2d::operator/(int32_t scalar) const noexcept {
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

vector2d &vector2d::operator*=(int32_t scalar) noexcept {
  _x *= scalar;
  _y *= scalar;
  return *this;
}

vector2d &vector2d::operator/=(int32_t scalar) noexcept {
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

int32_t vector2d::magnitude() const noexcept {
  return std::sqrt(_x * _x + _y * _y);
}

vector2d vector2d::unit() const noexcept {
  const auto m = magnitude();
  return (m < std::numeric_limits<int32_t>::epsilon()) ? vector2d(0, 0) : *this / m;
}

int32_t vector2d::dot(const vector2d &other) const noexcept {
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
