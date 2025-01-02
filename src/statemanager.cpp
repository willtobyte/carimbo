#include "statemanager.hpp"

#include "event.hpp"
#include <unordered_map>

using namespace framework;

bool statemanager::on(int player, const std::variant<input::controller> &type) const noexcept {
  auto pit = _state.find(player);
  if (pit != _state.end()) {
    auto &map = pit->second;
    auto tit = map.find(type);
    if (tit != map.end()) {
      return tit->second;
    }
  }

  return false;
}

constexpr std::optional<input::controller> keytoctrl(const input::keyevent &event) {
  using input::controller;
  using input::keyevent;

  switch (event) {
  case keyevent::up:
    return controller::up;
  case keyevent::down:
    return controller::down;
  case keyevent::left:
    return controller::left;
  case keyevent::right:
    return controller::right;
  case keyevent::space:
    return controller::cross;
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
