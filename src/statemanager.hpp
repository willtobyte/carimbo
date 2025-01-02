#pragma once

#include "common.hpp"

namespace framework {
class statemanager : public input::eventreceiver {
public:
  statemanager() = default;
  virtual ~statemanager() = default;

  // bool is_keydown(const input::keyevent &event) const;
  bool on(int player, const std::variant<input::controller> &type) const noexcept;

protected:
  virtual void on_keydown(const input::keyevent &event) noexcept;

  virtual void on_keyup(const input::keyevent &event) noexcept;

private:
  std::unordered_map<uint8_t, std::unordered_map<std::variant<input::controller>, bool>> _state;
};
}
