#pragma once

#include "common.hpp"
#include "eventreceiver.hpp"
#include "noncopyable.hpp"

typedef std::unique_ptr<SDL_Gamepad, SDL_Deleter> gamepad_ptr;

namespace input {
class eventmanager : private framework::noncopyable {
public:
  eventmanager();
  virtual ~eventmanager() = default;

  void update(float_t delta);

  void add_receiver(std::shared_ptr<eventreceiver> receiver) noexcept;

  void remove_receiver(std::shared_ptr<eventreceiver> receiver) noexcept;

private:
  std::list<std::shared_ptr<eventreceiver>> _receivers;
  std::unordered_map<SDL_JoystickID, gamepad_ptr> _controllers;
};
}
