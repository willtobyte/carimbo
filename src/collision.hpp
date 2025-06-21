#pragma once

#include "common.hpp"

namespace framework {
struct collision final {
  uint64_t a;
  uint64_t b;

  collision() noexcept;

  collision(uint64_t a, uint64_t b) noexcept;

  void reset() noexcept;

  void reset(uint64_t a, uint64_t b) noexcept;
};
}
