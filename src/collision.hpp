#pragma once

#include "common.hpp"

namespace framework {
struct collision {
  constexpr collision(uint64_t a, uint64_t b)
      : a(a), b(b) {}

  uint64_t a;
  uint64_t b;
};
}
