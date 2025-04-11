#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "collision.hpp"
#include "lifecycleobserver.hpp"

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

  bool on(int player, const std::variant<input::event::gamepad::button> &type) const noexcept;

  int8_t players() const noexcept;

protected:
  virtual void on_keydown(const input::event::keyboard::key &event) noexcept;

  virtual void on_keyup(const input::event::keyboard::key &event) noexcept;

  virtual void on_gamepadbuttondown(uint8_t who, const input::event::gamepad::button &event) noexcept;

  virtual void on_gamepadbuttonup(uint8_t who, const input::event::gamepad::button &event) noexcept;

  virtual void on_gamepadmotion(uint8_t who, const input::event::gamepad::motion &event) noexcept;

  virtual void on_collision(const input::event::collision &event) noexcept;

  virtual void on_endupdate() noexcept;

private:
  std::unordered_map<int8_t, std::unordered_map<std::variant<input::event::gamepad::button>, bool>> _state;

  std::unordered_map<std::pair<uint64_t, uint64_t>, bool, pairhash> _collision_mapping{1024};
};
}
