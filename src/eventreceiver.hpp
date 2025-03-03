#pragma once

#include "common.hpp"

#include "event.hpp"

namespace input {
class eventreceiver {
public:
  virtual ~eventreceiver() = default;

  virtual void on_quit() noexcept;
  virtual void on_keydown(const keyevent &event) noexcept;
  virtual void on_keyup(const keyevent &event) noexcept;
  virtual void on_joystickbuttondown(uint8_t who, const joystickevent &event) noexcept;
  virtual void on_joystickbuttonup(uint8_t who, const joystickevent &event) noexcept;
  virtual void on_joystickaxismotion(uint8_t who, const joystickaxisevent &event) noexcept;
  virtual void on_mail(const mailevent &event) noexcept;
  virtual void on_collision(const collisionevent &event) noexcept;
};
}
