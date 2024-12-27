#pragma once

#include "common.hpp"

namespace input {
enum eventtype : Uint32 {
  mail = SDL_USEREVENT + 1,
  timer
};

enum class keyevent : int32_t {
  up = SDLK_UP,
  left = SDLK_LEFT,
  down = SDLK_DOWN,
  right = SDLK_RIGHT,
  space = SDLK_SPACE,
};

enum class mouseevent : int32_t {};

enum class joystickevent : int32_t {
  dpad_up = SDL_CONTROLLER_BUTTON_DPAD_UP,
  dpad_down = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
  dpad_left = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  dpad_right = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
};

class mailevent {
public:
  mailevent(uint64_t to, const std::string &body) noexcept
      : to(to), body(body) {}

  uint64_t to;
  std::string body;
};
}
