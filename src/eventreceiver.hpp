#pragma once

#include "common.hpp"

#include "event.hpp"

namespace input {
class eventreceiver {
public:
  virtual ~eventreceiver() = default;

  virtual void on_quit() noexcept;
  virtual void on_keydown(const event::keyboard::key &event) noexcept;
  virtual void on_keyup(const event::keyboard::key &event) noexcept;
  virtual void on_mousebuttondown(const event::mouse::button &event) noexcept;
  virtual void on_mousebuttonup(const event::mouse::button &event) noexcept;
  virtual void on_mousemotion(const event::mouse::motion &event) noexcept;
  virtual void on_gamepadbuttondown(uint8_t who, const event::gamepad::button &event) noexcept;
  virtual void on_gamepadbuttonup(uint8_t who, const event::gamepad::button &event) noexcept;
  virtual void on_gamepadmotion(uint8_t who, const event::gamepad::motion &event) noexcept;
  virtual void on_mail(const event::mail &event) noexcept;
  virtual void on_collision(const event::collision &event) noexcept;
};
}
