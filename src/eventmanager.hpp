#pragma once

#include "common.hpp"

#include "noncopyable.hpp"

  class renderer;

  class eventreceiver;

class eventmanager final : private noncopyable {
public:
  explicit eventmanager(std::shared_ptr<renderer> renderer);
  virtual ~eventmanager() = default;

  void update(float delta);

  void add_receiver(const std::shared_ptr<eventreceiver>& receiver);

  void remove_receiver(const std::shared_ptr<eventreceiver>& receiver);

  void flush(uint32_t begin_event, uint32_t end_event = 0);

private:
  std::shared_ptr<renderer> _renderer;
  boost::container::small_vector<std::shared_ptr<eventreceiver>, 16> _receivers;
  boost::unordered_flat_map<uint32_t, std::unique_ptr<SDL_Gamepad, SDL_Deleter>> _controllers;
  boost::container::small_vector<uint32_t, 8> _joystickgorder;
  boost::unordered_flat_map<uint32_t, uint8_t> _joystickmapping;
};
