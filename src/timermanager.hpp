#pragma once

#include "common.hpp"

namespace framework {
class timermanager {
public:
  timermanager() = default;
  ~timermanager() noexcept;

  int32_t set(int32_t interval, std::function<void()> fn);
  int32_t singleshot(int32_t timeout, std::function<void()> fn);
  void clear(int32_t id) noexcept;

protected:
  int32_t add_timer(int32_t interval, std::function<void()> fn, bool repeat);

private:
  std::map<int32_t, std::shared_ptr<std::function<void()>>> _timers;
};
}
