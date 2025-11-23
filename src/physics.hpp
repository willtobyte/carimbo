#pragma once

#include "common.hpp"

namespace physics {
inline uint64_t userdata_to_id(void* userdata) {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(userdata));
}

inline void* id_to_userdata(uint64_t id) {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
}

enum collisioncategory : std::uint64_t {
  none        = 0u,
  player      = 1u << 0,  // 0x0001
  enemy       = 1u << 1,  // 0x0002
  projectile  = 1u << 2,  // 0x0004
  terrain     = 1u << 3,  // 0x0008
  trigger     = 1u << 4,  // 0x0010
  collectible = 1u << 5,  // 0x0020
  interface   = 1u << 6,  // 0x0040
  all         = 0xFFFFu
};
}
