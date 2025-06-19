#pragma once

#include "common.hpp"

namespace framework {
struct collision final {
  uint64_t a;
  uint64_t b;

  constexpr collision() noexcept
      : a(0), b(0) {}

  constexpr collision(uint64_t a, uint64_t b) noexcept
      : a(a), b(b) {}

  constexpr void reset() noexcept { a = 0; b = 0; }

  constexpr void reset(uint64_t a, uint64_t b) noexcept {
    this->a = a;
    this->b = b;
  }
};
}
