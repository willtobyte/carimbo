#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"

namespace framework {
struct pairhash final {
  size_t operator()(const std::pair<uint64_t, uint64_t> &p) const {
    const auto h1 = std::hash<uint64_t>{}(p.first);
    const auto h2 = std::hash<uint64_t>{}(p.second);
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

class statemanager : public input::eventreceiver, public lifecycleobserver {
public:
  statemanager();
  virtual ~statemanager() = default;

  bool collides(std::shared_ptr<object> a, std::shared_ptr<object> b) const;

  bool on(uint8_t player, const std::variant<input::event::gamepad::button> &type) const;

  uint8_t players() const;

protected:
  virtual void on_keydown(const input::event::keyboard::key &event);

  virtual void on_keyup(const input::event::keyboard::key &event);

  virtual void on_gamepadbuttondown(uint8_t who, const input::event::gamepad::button &event);

  virtual void on_gamepadbuttonup(uint8_t who, const input::event::gamepad::button &event);

  virtual void on_gamepadmotion(uint8_t who, const input::event::gamepad::motion &event);

  virtual void on_collision(const input::event::collision &event);

  virtual void on_endupdate();

private:
  std::unordered_map<uint8_t, std::unordered_map<std::variant<input::event::gamepad::button>, bool>> _state;

  std::unordered_map<std::pair<uint64_t, uint64_t>, bool, pairhash> _collision_mapping;
};
}
