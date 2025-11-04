#pragma once

#include "common.hpp"

namespace framework::physics {
inline uint64_t userdata_to_id(void* userdata) noexcept {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(userdata));
}

inline void* id_to_userdata(uint64_t id) noexcept {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
}

class body_transform final {
public:
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

private:
  float px{0.0f};
  float py{0.0f};
  float hx{0.0f};
  float hy{0.0f};
  float radians{0.0f};
};

}
