#include "statemanager.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"
#include "object.hpp"

statemanager::statemanager() {
}

bool statemanager::on(uint8_t player, event::gamepad::button type) const noexcept {
  if (const auto pit = _state.find(player); pit != _state.end()) {
    if (const auto tit = pit->second.find(type); tit != pit->second.end()) {
      return tit->second;
    }
  }

  return false;
}

uint8_t statemanager::players() const noexcept {
  return static_cast<uint8_t>(_state.size());
}

static constexpr inline std::optional<event::gamepad::button> keytoctrl(const event::keyboard::key& event) noexcept {

  switch (event) {
  case event::keyboard::key::up:
    return event::gamepad::button::up;
  case event::keyboard::key::down:
    return event::gamepad::button::down;
  case event::keyboard::key::left:
    return event::gamepad::button::left;
  case event::keyboard::key::right:
    return event::gamepad::button::right;
  case event::keyboard::key::space:
    return event::gamepad::button::south;
  default:
    return std::nullopt;
  }
}

void statemanager::on_key_press(const event::keyboard::key& event) {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = true;
  }
}

void statemanager::on_key_release(const event::keyboard::key& event) {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = false;
  }
}

void statemanager::on_gamepad_press(uint8_t who, const event::gamepad::button& event) {
  _state[who][event] = true;
}

void statemanager::on_gamepad_release(uint8_t who, const event::gamepad::button& event) {
  _state[who][event] = false;
}

void statemanager::on_gamepad_motion(uint8_t who, const event::gamepad::motion& event) {

  static constexpr auto threshold = 8000;
  static constexpr auto deadzone = 4000;
  const auto process = [&](event::keyboard::key negative, event::keyboard::key positive) {
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
  case event::gamepad::motion::axis::lefty:
    process(event::keyboard::key::up, event::keyboard::key::down);
    break;

  case event::gamepad::motion::axis::leftx:
    process(event::keyboard::key::left, event::keyboard::key::right);
    break;

  default:
    break;
  }
}

void statemanager::on_endupdate() noexcept {
}
