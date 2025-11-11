#pragma once

#include "common.hpp"

namespace framework::physics {
constexpr uint64_t userdata_to_id(void* userdata) {
  return std::bit_cast<uint64_t>(userdata);
}

constexpr void* id_to_userdata(uint64_t id) {
  return std::bit_cast<void*>(static_cast<uintptr_t>(id));
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

  float px{.0f};
  float py{.0f};
  float hx{.0f};
  float hy{.0f};
  float radians{.0f};
};

}
