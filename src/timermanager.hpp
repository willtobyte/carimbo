#pragma once

#include "common.hpp"

class timermanager final {
public:
  explicit timermanager();
  ~timermanager() noexcept;

  uint32_t set(uint32_t interval, sol::protected_function fn);
  uint32_t singleshot(uint32_t timeout, sol::protected_function fn);
  void cancel(uint32_t id) noexcept;
  void clear() noexcept;

protected:
  uint32_t add_timer(uint32_t interval, functor&& fn, bool repeat) noexcept;

private:
  envelopepool_impl& _envelopepool;

  boost::unordered_flat_map<uint32_t, envelope*> _envelopemapping;
};