#pragma once

#include "common.hpp"

namespace framework::physics {
inline uint64_t userdata_to_id(void* userdata) {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(userdata));
}

inline void* id_to_userdata(uint64_t id) {
  return reinterpret_cast<void*>(static_cast<uintptr_t>(id));
}

enum collisioncategory : uint64_t {
  player = B2_DEFAULT_CATEGORY_BITS << 0,      // 0x0001
  enemy = B2_DEFAULT_CATEGORY_BITS << 1,       // 0x0002
  projectile = B2_DEFAULT_CATEGORY_BITS << 2,  // 0x0004
  terrain = B2_DEFAULT_CATEGORY_BITS << 3,     // 0x0008
  trigger = B2_DEFAULT_CATEGORY_BITS << 4,     // 0x0010
  collectible = B2_DEFAULT_CATEGORY_BITS << 4, // 0x0020
  interface = B2_DEFAULT_CATEGORY_BITS << 8,   // 0x0040
  all = B2_DEFAULT_MASK_BITS                   // All
};
}
