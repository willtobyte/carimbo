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
  
  const auto center_x = bounds_width * 0.5f;
  const auto center_y = bounds_height * 0.5f;
  result.px = position_x + center_x;
  result.py = position_y + center_y;
  result.radians = static_cast<float>(angle_degrees * DEGREES_TO_RADIANS);

  return result;
}

bool body_transform::shape_differs(const body_transform& other) const noexcept {
  return std::abs(hx - other.hx) > epsilon ||
         std::abs(hy - other.hy) > epsilon;
}
