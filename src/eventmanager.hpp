#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "noncopyable.hpp"
#include "event.hpp"

namespace input {
class eventmanager : private framework::noncopyable {
public:
  explicit eventmanager(std::shared_ptr<graphics::renderer> renderer);
  virtual ~eventmanager() = default;

  void update(float_t delta);

  void add_receiver(std::shared_ptr<eventreceiver> receiver);

  void remove_receiver(std::shared_ptr<eventreceiver> receiver);

private:
  std::shared_ptr<graphics::renderer> _renderer;
  std::vector<std::shared_ptr<eventreceiver>> _receivers;
  std::unordered_map<SDL_JoystickID, std::unique_ptr<SDL_Gamepad, SDL_Deleter>> _controllers;
};
}
