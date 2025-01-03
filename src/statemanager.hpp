#pragma once

#include "common.hpp"

namespace framework {
class statemanager : public input::eventreceiver {
public:
  statemanager() = default;
  virtual ~statemanager() = default;

  // bool is_keydown(const input::keyevent &event) const;
  bool on(int player, const std::variant<input::joystickevent> &type) const noexcept;

protected:
  virtual void on_keydown(const input::keyevent &event) noexcept;

  virtual void on_keyup(const input::keyevent &event) noexcept;

  virtual void on_joystickbuttondown(int who, const input::joystickevent &event) noexcept;

  virtual void on_joystickbuttonup(int who, const input::joystickevent &event) noexcept;

private:
  std::unordered_map<uint8_t, std::unordered_map<std::variant<input::joystickevent>, bool>> _state;
};
}
