#include "vector2d.hpp"

using namespace algebra;

static constexpr float epsilon = std::numeric_limits<float>::epsilon();

vector2d::vector2d() noexcept
    : _x(0), _y(0) {}

vector2d::vector2d(float x, float y) noexcept
    : _x(x), _y(y) {}

float vector2d::x() const noexcept {
  return _x;
}

float vector2d::y() const noexcept {
  return _y;
}

void vector2d::set_x(float x) noexcept {
  _x = x;
}

void vector2d::set_y(float y) noexcept {
  _y = y;
}

void vector2d::set(float x, float y) noexcept {
  _x = x;
  _y = y;
}

vector2d vector2d::operator+(const vector2d& other) const noexcept {
  return vector2d(_x + other._x, _y + other._y);
}

vector2d vector2d::operator-(const vector2d& other) const noexcept {
  return vector2d(_x - other._x, _y - other._y);
}

vector2d vector2d::operator*(float scalar) const noexcept {
  return vector2d(_x * scalar, _y * scalar);
}

vector2d vector2d::operator/(float scalar) const noexcept {
  assert(std::abs(scalar) > std::numeric_limits<float>::epsilon());
  return vector2d(_x / scalar, _y / scalar);
}

vector2d& vector2d::operator+=(const vector2d& other) noexcept {
  _x += other._x;
  _y += other._y;
  return *this;
}

vector2d& vector2d::operator-=(const vector2d& other) noexcept {
  _x -= other._x;
  _y -= other._y;
  return *this;
}

vector2d& vector2d::operator*=(float scalar) noexcept {
  _x *= scalar;
  _y *= scalar;
  return *this;
}

vector2d& vector2d::operator/=(float scalar) noexcept {
  assert(std::abs(scalar) > std::numeric_limits<float>::epsilon());
  _x /= scalar;
  _y /= scalar;
  return *this;
}

bool vector2d::operator==(const vector2d& other) const noexcept {
  const auto tolerance = std::numeric_limits<float>::epsilon()
    * std::max({1.0f, std::abs(_x), std::abs(other._x), std::abs(_y), std::abs(other._y)});

  return std::abs(_x - other._x) < tolerance && std::abs(_y - other._y) < tolerance;
}

bool vector2d::operator!=(const vector2d& other) const noexcept {
  return !(*this == other);
}

float vector2d::magnitude() const noexcept {
  return std::sqrt(_x * _x + _y * _y);
}

vector2d vector2d::unit() const noexcept {
  const auto m = magnitude();
  return (m < std::numeric_limits<float>::epsilon()) ? vector2d(0, 0) : *this / m;
}

float vector2d::dot(const vector2d& other) const noexcept {
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
  return std::abs(_x) < epsilon && std::abs(_y) < epsilon;
}

vector2d vector2d::rotated(float angle) const noexcept {
  const auto c = std::cos(angle);
  const auto s = std::sin(angle);
  return vector2d(_x * c - _y * s, _x * s + _y * c);
}

float vector2d::cross(const vector2d& other) const noexcept {
  return _x * other._y - _y * other._x;
}

void vector2d::normalize() noexcept {
  const auto m = magnitude();
  if (m >= epsilon) {
    _x /= m;
    _y /= m;
  }
}

float vector2d::angle_between(const vector2d& other) const noexcept {
  float dot = this->dot(other);
  float ms = this->magnitude() * other.magnitude();
  return (ms < epsilon) ? 0.0f : std::acos(dot / ms);
}

vector2d vector2d::clamped(float max) const noexcept {
  const auto m = magnitude();
  return (m > max && m > epsilon) ? *this * (max / m) : *this;
}

float vector2d::distance_to(const vector2d& other) const noexcept {
  const auto dx = _x - other._x;
  const auto dy = _y - other._y;
  return std::sqrt(dx * dx + dy * dy);
}
