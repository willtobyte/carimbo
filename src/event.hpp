#pragma once

#include "common.hpp"

namespace input {
enum player : uint8_t {
  one = 0,
  two
};

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

enum class joystickevent : int32_t {
  up = SDL_CONTROLLER_BUTTON_DPAD_UP,
  down = SDL_CONTROLLER_BUTTON_DPAD_DOWN,
  left = SDL_CONTROLLER_BUTTON_DPAD_LEFT,
  right = SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
  triangle = SDL_CONTROLLER_BUTTON_Y,
  circle = SDL_CONTROLLER_BUTTON_B,
  cross = SDL_CONTROLLER_BUTTON_A,
  square = SDL_CONTROLLER_BUTTON_X,
};

enum class mouseevent : int32_t {};

class mailevent {
public:
  mailevent(uint64_t to, const std::string &body) noexcept
      : to(to), body(body) {}

  uint64_t to;
  std::string body;
};
}
