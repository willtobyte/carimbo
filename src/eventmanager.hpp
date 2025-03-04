#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "noncopyable.hpp"
#include "postalservice.hpp"

namespace input {
class eventmanager : private framework::noncopyable {
public:
  eventmanager();
  virtual ~eventmanager() = default;

  void update(float_t delta);

  void add_receiver(std::shared_ptr<eventreceiver> receiver) noexcept;

  void remove_receiver(std::shared_ptr<eventreceiver> receiver) noexcept;

private:
  std::vector<std::shared_ptr<eventreceiver>> _receivers;
  std::unordered_map<SDL_JoystickID, std::unique_ptr<SDL_GameController, SDL_Deleter>> _controllers;
};
}
