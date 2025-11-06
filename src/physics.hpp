#pragma once

#include "common.hpp"

namespace framework::physics {
inline uint64_t userdata_to_id(void* userdata) {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(userdata));
}

inline void* id_to_userdata(uint64_t id) {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
}

enum collisioncategory : uint32_t {
  Player = 0x0001,
  Enemy = 0x0002,
  Projectile = 0x0004,
  Terrain = 0x0008,
  Trigger = 0x0010,
};

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
  );

  bool shape_differs(const body_transform& other) const;
  bool rotation_differs(const body_transform& other) const;
  bool differs(const body_transform& other) const;

  float px{.0f};
  float py{.0f};
  float hx{.0f};
  float hy{.0f};
  float radians{.0f};
};

}
