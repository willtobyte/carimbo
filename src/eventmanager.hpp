#pragma once

#include "common.hpp"

#include "envelope.hpp"
#include "eventreceiver.hpp"
#include "objectpool.hpp"
#include "noncopyable.hpp"

namespace input {
class eventmanager final : private framework::noncopyable {
public:
  explicit eventmanager(std::shared_ptr<graphics::renderer> renderer);
  virtual ~eventmanager() noexcept = default;

  void update(float_t delta) noexcept;

  void add_receiver(std::shared_ptr<eventreceiver> receiver);

  void remove_receiver(std::shared_ptr<eventreceiver> receiver);

private:
  std::shared_ptr<graphics::renderer> _renderer;
  std::vector<std::shared_ptr<eventreceiver>> _receivers;
  std::unordered_map<SDL_JoystickID, std::unique_ptr<SDL_Gamepad, SDL_Deleter>> _controllers;
  std::unordered_map<SDL_JoystickID, uint8_t> _joystickmapping;
  std::vector<SDL_JoystickID> _mappingorder;
  std::shared_ptr<framework::uniquepool<framework::envelope>> _envelopepool;
};
}
