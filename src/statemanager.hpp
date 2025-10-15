#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"

namespace framework {
class statemanager final : public input::eventreceiver, public lifecycleobserver {
public:
  statemanager() noexcept;
  virtual ~statemanager() noexcept = default;

  bool collides(std::shared_ptr<object> a, std::shared_ptr<object> b) const noexcept;

  bool on(uint8_t player, const input::event::gamepad::button type) const noexcept;

  uint8_t players() const noexcept;

protected:
  virtual void on_key_press(const input::event::keyboard::key& event) override;

  virtual void on_key_release(const input::event::keyboard::key& event) override;

  virtual void on_gamepad_press(uint8_t who, const input::event::gamepad::button& event) override;

  virtual void on_gamepad_release(uint8_t who, const input::event::gamepad::button& event) override;

  virtual void on_gamepad_motion(uint8_t who, const input::event::gamepad::motion& event) override;

  virtual void on_collision(const input::event::collision& event) override;

  virtual void on_endupdate() override;

private:
  std::unordered_map<uint8_t, std::unordered_map<input::event::gamepad::button, bool>> _state;

  boost::unordered_map<std::pair<uint64_t, uint64_t>, bool, boost::hash<std::pair<uint64_t, uint64_t>>> _collision_mapping;
};
}
