#pragma once

#include "common.hpp"

#include "event.hpp"

namespace input {
class eventreceiver {
public:
  virtual ~eventreceiver() noexcept = default;

  virtual void on_quit();
  virtual void on_key_press(const event::keyboard::key &event);
  virtual void on_key_release(const event::keyboard::key &event);
  virtual void on_text(const std::string &text);
  virtual void on_mouse_press(const event::mouse::button &event);
  virtual void on_mouse_release(const event::mouse::button &event);
  virtual void on_mouse_motion(const event::mouse::motion &event);
  virtual void on_gamepad_press(uint8_t who, const event::gamepad::button &event);
  virtual void on_gamepad_release(uint8_t who, const event::gamepad::button &event);
  virtual void on_gamepad_motion(uint8_t who, const event::gamepad::motion &event);
  virtual void on_mail(const event::mail &event);
  virtual void on_collision(const event::collision &event);
};
}
