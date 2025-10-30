#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"

namespace framework {
struct collision_hash final {
  std::size_t operator()(const std::pair<uint64_t, uint64_t>& p) const noexcept {
    const auto h1 = std::hash<uint64_t>{}(p.first);
    const auto h2 = std::hash<uint64_t>{}(p.second);
    return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
  }
};

struct collision_equal final {
  bool operator()(const std::pair<uint64_t, uint64_t>& lhs,
                  const std::pair<uint64_t, uint64_t>& rhs) const noexcept {
    return (lhs.first == rhs.first && lhs.second == rhs.second) ||
            (lhs.first == rhs.second && lhs.second == rhs.first);
  }
};

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

  boost::unordered_flat_set<std::pair<uint64_t, uint64_t>, collision_hash, collision_equal> _collision_mapping;
};
}
