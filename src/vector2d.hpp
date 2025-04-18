#pragma once

#include "common.hpp"

namespace algebra {
class vector2d {
public:
  vector2d() noexcept;
  vector2d(float_t x, float_t y) noexcept;

  float_t x() const noexcept;
  float_t y() const noexcept;

  void set_x(float_t x) noexcept;
  void set_y(float_t y) noexcept;
  void set(float_t x, float_t y) noexcept;

  vector2d operator+(const vector2d &other) const noexcept;
  vector2d operator-(const vector2d &other) const noexcept;
  vector2d operator*(float_t scalar) const noexcept;
  vector2d operator/(float_t scalar) const noexcept;

  vector2d &operator+=(const vector2d &other) noexcept;
  vector2d &operator-=(const vector2d &other) noexcept;
  vector2d &operator*=(float_t scalar) noexcept;
  vector2d &operator/=(float_t scalar) noexcept;

  bool operator==(const vector2d &other) const noexcept;
  bool operator!=(const vector2d &other) const noexcept;

  float_t magnitude() const noexcept;

  vector2d unit() const noexcept;

  float_t dot(const vector2d &other) const noexcept;

  bool moving() const noexcept;
  bool right() const noexcept;
  bool left() const noexcept;
  bool zero() const noexcept;

private:
  float_t _x;
  float_t _y;
};
}
