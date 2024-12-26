#pragma once

#include "common.hpp"

namespace algebra {
class vector2d {
public:
  vector2d() noexcept;
  vector2d(int32_t x, int32_t y) noexcept;

  int32_t x() const noexcept;
  int32_t y() const noexcept;

  void set_x(int32_t x) noexcept;
  void set_y(int32_t y) noexcept;
  void set(int32_t x, int32_t y) noexcept;

  vector2d operator+(const vector2d &other) const noexcept;
  vector2d operator-(const vector2d &other) const noexcept;
  vector2d operator*(int32_t scalar) const noexcept;
  vector2d operator/(int32_t scalar) const noexcept;

  vector2d &operator+=(const vector2d &other) noexcept;
  vector2d &operator-=(const vector2d &other) noexcept;
  vector2d &operator*=(int32_t scalar) noexcept;
  vector2d &operator/=(int32_t scalar) noexcept;

  bool operator==(const vector2d &other) const noexcept;
  bool operator!=(const vector2d &other) const noexcept;

  int32_t magnitude() const noexcept;

  vector2d unit() const noexcept;

  int32_t dot(const vector2d &other) const noexcept;

  bool moving() const noexcept;
  bool right() const noexcept;
  bool left() const noexcept;
  bool zero() const noexcept;

private:
  int32_t _x;
  int32_t _y;
};
}
