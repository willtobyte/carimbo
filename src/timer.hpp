#pragma once

#include "common.hpp"

namespace framework {
struct timer final {
  bool repeat{false};
  std::function<void()> fn;

  timer() noexcept = default;

  timer(bool repeat, std::function<void()> fn) noexcept
      : repeat(repeat), fn(std::move(fn)) {}

  void reset() noexcept { repeat = false; fn = nullptr; }

  void reset(bool repeat, std::function<void()> fn) noexcept {
    this->repeat = repeat;
    this->fn = fn;
  }
};
}
