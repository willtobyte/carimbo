#pragma once

#include "common.hpp"

namespace framework {
class timermanager final {
public:
  explicit timermanager() = default;
  ~timermanager() = default;

  int32_t set(int32_t interval, std::function<void()> fn);
  int32_t singleshot(int32_t timeout, std::function<void()> fn);
  void clear(int32_t id);

protected:
  int32_t add_timer(int32_t interval, std::function<void()> fn, bool repeat);
};
}
