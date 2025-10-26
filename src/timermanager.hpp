#pragma once

#include "common.hpp"

namespace framework {
class timermanager final {
public:
  explicit timermanager() noexcept;
  ~timermanager() noexcept = default;

  uint32_t set(uint32_t interval, std::function<void()>&& fn) noexcept;
  uint32_t singleshot(uint32_t timeout, std::function<void()>&& fn) noexcept;
  void cancel(uint32_t id) noexcept;
  void clear() noexcept;

  void set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager) noexcept;

protected:
  uint32_t add_timer(uint32_t interval, std::function<void()>&& fn, bool repeat);

private:
  std::shared_ptr<uniquepool<envelope, envelope_pool_name>> _envelopepool;

  std::unordered_map<uint32_t, envelope*> _envelopemapping;
  
  std::shared_ptr<input::eventmanager> _eventmanager;
};
}
