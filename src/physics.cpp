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
) {
  body_transform result;

  const auto sw = bounds_width * scale;
  const auto sh = bounds_height * scale;
  result._hx = 0.5f * sw;
  result._hy = 0.5f * sh;
  
  const auto center_x = bounds_width * 0.5f;
  const auto center_y = bounds_height * 0.5f;
  result._px = position_x + center_x;
  result._py = position_y + center_y;
  result._radians = static_cast<float>(angle_degrees * DEGREES_TO_RADIANS);

  return result;
}

