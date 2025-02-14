#pragma once

#include "common.hpp"

namespace input {
class eventreceiver {
public:
  virtual ~eventreceiver() = default;

  virtual void on_quit() noexcept;
  virtual void on_keydown(const keyevent &event) noexcept;
  virtual void on_keyup(const keyevent &event) noexcept;
  virtual void on_joystickbuttondown(int who, const joystickevent &event) noexcept;
  virtual void on_joystickbuttonup(int who, const joystickevent &event) noexcept;
  virtual void on_joystickaxismotion(int who, const joystickaxisevent &event) noexcept;
  virtual void on_mail(const mailevent &event) noexcept;
  virtual void on_collision(const collisionevent &event) noexcept;
};
}
