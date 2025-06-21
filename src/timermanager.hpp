#pragma once

#include "common.hpp"

namespace framework {
class timermanager final {
public:
  explicit timermanager() noexcept;
  ~timermanager() noexcept = default;

  uint32_t set(uint32_t interval, std::function<void()> fn);
  uint32_t singleshot(uint32_t timeout, std::function<void()> fn);
  void clear(uint32_t id);

protected:
  uint32_t add_timer(uint32_t interval, std::function<void()> fn, bool repeat);

private:
  std::shared_ptr<uniquepool<timer>> _timerpool;
};
}
