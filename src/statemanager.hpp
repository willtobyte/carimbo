#pragma once

#include "common.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"
#include "object.hpp"

namespace framework {
struct pairhash {
  size_t operator()(const std::pair<uint64_t, uint64_t> &p) const noexcept {
    const auto h1 = std::hash<uint64_t>{}(p.first);
    const auto h2 = std::hash<uint64_t>{}(p.second);
    return h1 ^ (h2 << 1);
  }
};

class statemanager : public input::eventreceiver, public lifecycleobserver {
public:
  statemanager() = default;
  virtual ~statemanager() = default;

  bool collides(std::shared_ptr<object> a, std::shared_ptr<object> b) const noexcept;

  bool on(int player, const std::variant<input::joystickevent> &type) const noexcept;

  int8_t players() const noexcept;

protected:
  virtual void on_keydown(const input::keyevent &event) noexcept;

  virtual void on_keyup(const input::keyevent &event) noexcept;

  virtual void on_joystickbuttondown(uint8_t who, const input::joystickevent &event) noexcept;

  virtual void on_joystickbuttonup(uint8_t who, const input::joystickevent &event) noexcept;

  virtual void on_joystickaxismotion(uint8_t who, const input::joystickaxisevent &event) noexcept;

  virtual void on_collision(const collisionevent &event) noexcept;

  virtual void on_endupdate() noexcept;

private:
  std::unordered_map<int8_t, std::unordered_map<std::variant<input::joystickevent>, bool>> _state;

  std::unordered_map<std::pair<uint64_t, uint64_t>, bool, pairhash> _collision_mapping{1024};
};
}
