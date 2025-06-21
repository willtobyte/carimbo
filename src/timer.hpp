#pragma once

#include "common.hpp"

namespace framework {
struct timer final {
  bool repeat;
  std::function<void()> fn;

  timer() noexcept;

  timer(bool repeat, std::function<void()> fn) noexcept;

  void reset() noexcept;

  void reset(bool repeat, std::function<void()> fn) noexcept;
};
}
