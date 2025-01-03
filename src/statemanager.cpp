#include "statemanager.hpp"

#include "event.hpp"
#include <unordered_map>

using namespace framework;

bool statemanager::on(int player, const std::variant<input::joystickevent> &type) const noexcept {
  if (const auto pit = _state.find(player); pit != _state.end()) {
    if (const auto tit = pit->second.find(type); tit != pit->second.end()) {
      return tit->second;
    }
  }

  return false;
}

constexpr std::optional<input::joystickevent> keytoctrl(const input::keyevent &event) {
  using input::joystickevent;
  using input::keyevent;

  switch (event) {
  case keyevent::up:
    return joystickevent::up;
  case keyevent::down:
    return joystickevent::down;
  case keyevent::left:
    return joystickevent::left;
  case keyevent::right:
    return joystickevent::right;
  case keyevent::space:
    return joystickevent::cross;
  default:
    return std::nullopt;
  }
}

void statemanager::on_keydown(const input::keyevent &event) noexcept {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = true;
  }
}

void statemanager::on_keyup(const input::keyevent &event) noexcept {
  if (auto ctrl = keytoctrl(event)) {
    _state[0][*ctrl] = false;
  }
}

void statemanager::on_joystickbuttondown(int who, const input::joystickevent &event) noexcept {
  _state[who][event] = true;
}

void statemanager::on_joystickbuttonup(int who, const input::joystickevent &event) noexcept {
  _state[who][event] = false;
}
