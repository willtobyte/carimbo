#pragma once

#include "common.hpp"

namespace framework {
class statemanager final : public input::eventreceiver, public lifecycleobserver {
public:
  statemanager();
  virtual ~statemanager() = default;

  bool on(uint8_t player, const input::event::gamepad::button type) const;

  uint8_t players() const;

protected:
  virtual void on_key_press(const input::event::keyboard::key& event) override;

  virtual void on_key_release(const input::event::keyboard::key& event) override;

  virtual void on_gamepad_press(uint8_t who, const input::event::gamepad::button& event) override;

  virtual void on_gamepad_release(uint8_t who, const input::event::gamepad::button& event) override;

  virtual void on_gamepad_motion(uint8_t who, const input::event::gamepad::motion& event) override;

  virtual void on_endupdate() override;

private:
  std::unordered_map<uint8_t, std::unordered_map<input::event::gamepad::button, bool>> _state;
};
}
