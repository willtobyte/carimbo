#pragma once

#include "common.hpp"

namespace framework::physics {

inline constexpr float DEG_TO_RAD = std::numbers::pi_v<float> / 180.0f;
inline constexpr float RAD_TO_DEG = 180.0f / std::numbers::pi_v<float>;

uint64_t userdata_to_id(void* userdata) noexcept;
void* id_to_userdata(uint64_t id) noexcept;

struct body_transform final {
  float px{0.0f};
  float py{0.0f};
  float hx{0.0f};
  float hy{0.0f};
  float radians{0.0f};

  body_transform() = default;

  static body_transform compute(
    float position_x,
    float position_y,
    float bounds_x,
    float bounds_y,
    float bounds_width,
    float bounds_height,
    float scale,
    double angle_degrees
  ) noexcept;

  bool shape_differs(const body_transform& other) const noexcept;
  bool rotation_differs(const body_transform& other) const noexcept;
  bool differs(const body_transform& other) const noexcept;
};

}
