#pragma once

#include "common.hpp"

class timermanager final {
public:
  explicit timermanager();
  ~timermanager() = default;

  uint32_t set(uint32_t interval, sol::protected_function fn);
  uint32_t singleshot(uint32_t timeout, sol::protected_function fn);
  void cancel(uint32_t id);
  void clear();

protected:
  uint32_t add_timer(uint32_t interval, std::function<void()>&& fn, bool repeat) noexcept;

private:
  std::shared_ptr<envelopepool_impl> _envelopepool;

  std::unordered_map<uint32_t, envelope*> _envelopemapping;
};
