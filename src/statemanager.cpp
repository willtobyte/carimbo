#include "statemanager.hpp"

using namespace framework;
using namespace input::event;

static constexpr inline std::pair<uint64_t, uint64_t> make_key(uint64_t a, uint64_t b) noexcept {
  return (a <= b) ? std::make_pair(a, b) : std::make_pair(b, a);
}

statemanager::statemanager() noexcept {
  _collision_mapping.reserve(1000);
}

bool statemanager::collides(std::shared_ptr<object> a, std::shared_ptr<object> b) const noexcept {
  auto it = _collision_mapping.find(make_key(a->id(), b->id()));
  return (it != _collision_mapping.end()) ? it->second : false;
}

bool statemanager::on(uint8_t player, gamepad::button type) const noexcept {
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

constexpr std::optional<input::event::gamepad::button> keytoctrl(const keyboard::key& event) {
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
    _state[1][*ctrl] = true;
  }
}

void statemanager::on_key_release(const keyboard::key& event) {
  if (auto ctrl = keytoctrl(event)) {
    _state[1][*ctrl] = false;
  }
}

void statemanager::on_gamepad_press(uint8_t who, const gamepad::button& event) {
  _state[who - 1][event] = true;
}

void statemanager::on_gamepad_release(uint8_t who, const gamepad::button& event) {
  _state[who - 1][event] = false;
}

void statemanager::on_gamepad_motion(uint8_t who, const gamepad::motion& event) {
  using namespace input;

  static constexpr auto threshold = 8000;
  static constexpr auto deadzone = 4000;
  const auto process = [&](keyboard::key negative, keyboard::key positive) {
    if (event.value < -threshold) {
      if (auto ctrl = keytoctrl(negative)) {
        _state[who - 1][*ctrl] = true;
      }
    } else if (event.value > threshold) {
      if (auto ctrl = keytoctrl(positive)) {
        _state[who - 1][*ctrl] = true;
      }
    } else if (std::abs(event.value) < deadzone) {
      if (auto ctrl = keytoctrl(negative)) {
        _state[who - 1][*ctrl] = false;
      }
      if (auto ctrl = keytoctrl(positive)) {
        _state[who - 1][*ctrl] = false;
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
  _collision_mapping[make_key(event.a, event.b)] = true;
}

void statemanager::on_endupdate() {
  _collision_mapping.clear();
}
