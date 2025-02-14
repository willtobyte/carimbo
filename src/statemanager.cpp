#include "statemanager.hpp"

#include "entity.hpp"
#include "event.hpp"

using namespace framework;

statemanager::statemanager()
    : _collision_map(128, [](const std::pair<uint64_t, uint64_t> &p) -> size_t {
        size_t h1 = std::hash<uint64_t>{}(p.first);
        size_t h2 = std::hash<uint64_t>{}(p.second);
        return h1 ^ (h2 << 1);
      }) {}

static const auto make_key = [](const std::shared_ptr<entity> &a, const std::shared_ptr<entity> &b) {
  return (a->id() <= b->id()) ? std::make_pair(a->id(), b->id())
                              : std::make_pair(b->id(), a->id());
};

bool statemanager::collides(const std::shared_ptr<entity> &a, const std::shared_ptr<entity> &b) const noexcept {
  auto it = _collision_map.find(make_key(a, b));
  return (it != _collision_map.end()) ? it->second : false;
}

bool statemanager::on(int player, const std::variant<input::joystickevent> &type) const noexcept {
  if (const auto pit = _state.find(player); pit != _state.end()) {
    if (const auto tit = pit->second.find(type); tit != pit->second.end()) {
      return tit->second;
    }
  }

  return false;
}

int8_t statemanager::players() const noexcept {
  return _state.size();
}

constexpr std::optional<input::joystickevent> keytoctrl(const input::keyevent &event) {
  using namespace input;

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

void statemanager::on_joystickaxismotion(int who, const input::joystickaxisevent &event) noexcept {
  using namespace input;

  static constexpr auto threshold = 8000;
  static constexpr auto deadzone = 4000;

  const auto process = [&](keyevent negative, keyevent positive) {
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
  case joystickaxisevent::axis::lefty:
    process(keyevent::up, keyevent::down);
    break;

  case joystickaxisevent::axis::leftx:
    process(keyevent::left, keyevent::right);
    break;

  default:
    break;
  }
}

void statemanager::on_collision(const collisionevent &event) noexcept {
  std::cout << "a " << event.a << " b " << event.b << std::endl;
  UNUSED(event);
}
