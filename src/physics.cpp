#include "physics.hpp"

using namespace framework::physics;

body_transform body_transform::compute(
  float position_x,
  float position_y,
  float bounds_x,
  float bounds_y,
  float bounds_width,
  float bounds_height,
  float scale,
  double angle_degrees
) noexcept {
  body_transform result;

  const auto sw = bounds_width * scale;
  const auto sh = bounds_height * scale;
  result.hx = 0.5f * sw;
  result.hy = 0.5f * sh;
  result.px = position_x + bounds_x * scale + result.hx;
  result.py = position_y + bounds_y * scale + result.hy;
  result.radians = static_cast<float>(angle_degrees * DEGREES_TO_RADIANS);

  return result;
}

bool body_transform::shape_differs(const body_transform& other) const noexcept {
  return std::abs(hx - other.hx) > epsilon ||
         std::abs(hy - other.hy) > epsilon;
}

bool body_transform::rotation_differs(const body_transform& other) const noexcept {
  return std::abs(radians - other.radians) > epsilon;
}

bool body_transform::differs(const body_transform& other) const noexcept {
  return shape_differs(other) || rotation_differs(other);
}
