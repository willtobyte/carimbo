#pragma once

#include "common.hpp"

namespace algebra {
class vector2d final {
public:
  vector2d();
  vector2d(float_t x, float_t y);
  ~vector2d() = default;

  float_t x() const;
  float_t y() const;

  void set_x(float_t x);
  void set_y(float_t y);
  void set(float_t x, float_t y);

  vector2d operator+(const vector2d &other) const;
  vector2d operator-(const vector2d &other) const;
  vector2d operator*(float_t scalar) const;
  vector2d operator/(float_t scalar) const;

  vector2d &operator+=(const vector2d &other);
  vector2d &operator-=(const vector2d &other);
  vector2d &operator*=(float_t scalar);
  vector2d &operator/=(float_t scalar);

  bool operator==(const vector2d &other) const;
  bool operator!=(const vector2d &other) const;

  float_t magnitude() const;

  vector2d unit() const;

  float_t dot(const vector2d &other) const;

  bool moving() const;
  bool right() const;
  bool left() const;
  bool zero() const;

  vector2d rotated(float_t angle) const;
  float_t cross(const vector2d &other) const;
  void normalize();
  float_t angle_between(const vector2d &other) const;
  vector2d clamped(float_t max) const;

  float_t distance_to(const vector2d &other) const;

private:
  float_t _x{.0f};
  float_t _y{.0f};
};
}
