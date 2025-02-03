#pragma once

#include "common.hpp"
#include "entity.hpp"

namespace framework {
class statemanager : public input::eventreceiver {
public:
  statemanager();
  virtual ~statemanager() = default;

  bool collides(const std::shared_ptr<entity> &a, const std::shared_ptr<entity> &b) const noexcept;

  bool on(int player, const std::variant<input::joystickevent> &type) const noexcept;

  int8_t players() const noexcept;

protected:
  virtual void on_keydown(const input::keyevent &event) noexcept;

  virtual void on_keyup(const input::keyevent &event) noexcept;

  virtual void on_joystickbuttondown(int who, const input::joystickevent &event) noexcept;

  virtual void on_joystickbuttonup(int who, const input::joystickevent &event) noexcept;

  virtual void on_joystickaxismotion(int who, const input::joystickaxisevent &event) noexcept;

private:
  std::unordered_map<int8_t, std::unordered_map<std::variant<input::joystickevent>, bool>> _state;

  std::unordered_map<std::pair<uint64_t, uint64_t>, bool, std::function<size_t(const std::pair<uint64_t, uint64_t> &)>> _collision_map;
};
}
