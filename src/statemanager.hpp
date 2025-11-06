#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"

namespace framework {
struct collision_pair {
  uint64_t a, b;

  collision_pair(uint64_t x, uint64_t y)
    : a(std::min(x, y)), b(std::max(x, y)) {}

  bool operator==(const collision_pair& other) const = default;
};

struct collision_hash final {
  using is_transparent = void;

  std::size_t operator()(const collision_pair& p) const {
    return std::hash<uint64_t>{}(p.a) ^ (std::hash<uint64_t>{}(p.b) << 1);
  }

  std::size_t operator()(std::pair<uint64_t, uint64_t> p) const {
    auto normalized = std::minmax(p.first, p.second);
    return std::hash<uint64_t>{}(normalized.first) ^ (std::hash<uint64_t>{}(normalized.second) << 1);
  }
};

struct collision_equal final {
  using is_transparent = void;

  bool operator()(const collision_pair& lhs, const collision_pair& rhs) const {
    return lhs.a == rhs.a && lhs.b == rhs.b;
  }

  bool operator()(std::pair<uint64_t, uint64_t> lhs, const collision_pair& rhs) const {
    auto [a, b] = std::minmax(lhs.first, lhs.second);
    return a == rhs.a && b == rhs.b;
  }

  bool operator()(const collision_pair& lhs, std::pair<uint64_t, uint64_t> rhs) const {
    return operator()(rhs, lhs);
  }
};

class statemanager final : public input::eventreceiver, public lifecycleobserver {
public:
  statemanager();
  virtual ~statemanager() = default;

  bool collides(const std::shared_ptr<object>& a, const std::shared_ptr<object>& b) const;

  bool on(uint8_t player, const input::event::gamepad::button type) const;

  uint8_t players() const;

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

  std::unordered_set<collision_pair, collision_hash, collision_equal> _collision_mapping;
};
}
