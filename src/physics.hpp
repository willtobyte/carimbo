#pragma once

#include "common.hpp"

enum class collisioncategory : std::uint64_t {
  none = 0ull,
  player = 1ull << 0,
  enemy = 1ull << 1,
  projectile = 1ull << 2,
  terrain = 1ull << 3,
  trigger = 1ull << 4,
  collectible = 1ull << 5,
  interface = 1ull << 6,
  all = ~0ull 
};
