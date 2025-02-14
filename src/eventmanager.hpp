#pragma once

#include "collision.hpp"
#include "common.hpp"
#include "event.hpp"
#include "eventreceiver.hpp"
#include "helpers.hpp"
#include "noncopyable.hpp"
#include "postalservice.hpp"

typedef std::unique_ptr<SDL_GameController, SDL_Deleter> gamecontroller_ptr;

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
  std::unordered_map<SDL_JoystickID, gamecontroller_ptr> _controllers;
};
}
