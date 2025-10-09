#pragma once

#include "common.hpp"

namespace algebra {
class vector2d final {
public:
  vector2d() noexcept;
  vector2d(float x, float y) noexcept;
  ~vector2d() noexcept = default;

  float x() const noexcept;
  float y() const noexcept;

  void set_x(float x) noexcept;
  void set_y(float y) noexcept;
  void set(float x, float y) noexcept;

  vector2d operator+(const vector2d& other) const noexcept;
  vector2d operator-(const vector2d& other) const noexcept;
  vector2d operator*(float scalar) const noexcept;
  vector2d operator/(float scalar) const noexcept;

  vector2d& operator+=(const vector2d& other) noexcept;
  vector2d& operator-=(const vector2d& other) noexcept;
  vector2d& operator*=(float scalar) noexcept;
  vector2d& operator/=(float scalar) noexcept;

  bool operator==(const vector2d& other) const noexcept;
  bool operator!=(const vector2d& other) const noexcept;

  float magnitude() const noexcept;

  vector2d unit() const noexcept;

  float dot(const vector2d& other) const noexcept;

  bool moving() const noexcept;
  bool right() const noexcept;
  bool left() const noexcept;
  bool zero() const noexcept;

  vector2d rotated(float angle) const noexcept;
  float cross(const vector2d& other) const noexcept;
  void normalize() noexcept;
  float angle_between(const vector2d& other) const noexcept ;
  vector2d clamped(float max) const noexcept;

  float distance_to(const vector2d& other) const noexcept;

private:
  float _x{.0f};
  float _y{.0f};
};
}
