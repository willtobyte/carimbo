#include "statemanager.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"
#include "object.hpp"

using namespace framework;
using namespace input::event;

statemanager::statemanager() {
  _collision_mapping.reserve(64);
}

bool statemanager::collides(const std::shared_ptr<object>& a, const std::shared_ptr<object>& b) const {
  return _collision_mapping.contains(std::pair{a->id(), b->id()});
}

bool statemanager::on(uint8_t player, gamepad::button type) const {
  if (const auto pit = _state.find(player); pit != _state.end()) {
    if (const auto tit = pit->second.find(type); tit != pit->second.end()) {
      return tit->second;
    }
  }

  return false;
}

uint8_t statemanager::players() const {
  return static_cast<uint8_t>(_state.size());
}

static constexpr inline std::optional<input::event::gamepad::button> keytoctrl(const keyboard::key& event) {
  using namespace input;

  switch (event) {
  case keyboard::key::up:
    return gamepad::button::up;
  case keyboard::key::down:
    return gamepad::button::down;
  case keyboard::key::left:
    return gamepad::button::left;
  case keyboard::key::right:
    return gamepad::button::right;
  case keyboard::key::space:
    return gamepad::button::south;
  default:
    return std::nullopt;
  }
}

void statemanager::on_key_press(const keyboard::key& event) {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = true;
  }
}

void statemanager::on_key_release(const keyboard::key& event) {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = false;
  }
}

void statemanager::on_gamepad_press(uint8_t who, const gamepad::button& event) {
  _state[who][event] = true;
}

void statemanager::on_gamepad_release(uint8_t who, const gamepad::button& event) {
  _state[who][event] = false;
}

void statemanager::on_gamepad_motion(uint8_t who, const gamepad::motion& event) {
  using namespace input;

  static constexpr auto threshold = 8000;
  static constexpr auto deadzone = 4000;
  const auto process = [&](keyboard::key negative, keyboard::key positive) {
    if (event.value < -threshold) {
      if (auto ctrl = keytoctrl(negative)) {
        _state[who][*ctrl] = true;
      }
    } else if (event.value > threshold) {
      if (auto ctrl = keytoctrl(positive)) {
        _state[who][*ctrl] = true;
      }
    } else if (std::abs(event.value) < deadzone) {
      if (auto ctrl = keytoctrl(negative)) {
        _state[who][*ctrl] = false;
      }
      if (auto ctrl = keytoctrl(positive)) {
        _state[who][*ctrl] = false;
      }
    }
  };

  switch (event.kind) {
  case gamepad::motion::axis::lefty:
    process(keyboard::key::up, keyboard::key::down);
    break;

  case gamepad::motion::axis::leftx:
    process(keyboard::key::left, keyboard::key::right);
    break;

  default:
    break;
  }
}

void statemanager::on_collision(const input::event::collision& event) {
  _collision_mapping.emplace(event.a, event.b);
}

void statemanager::on_endupdate() {
  _collision_mapping.clear();
}
