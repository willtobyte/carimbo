#pragma once

#include "common.hpp"

namespace input {
enum player : uint8_t {
  one = 0,
  two
};

enum eventtype : uint32_t {
  collision = SDL_EVENT_USER + 1,
  mail,
  timer
};

enum class keyevent : int32_t {
  up = SDLK_UP,
  left = SDLK_LEFT,
  down = SDLK_DOWN,
  right = SDLK_RIGHT,
  space = SDLK_SPACE,
};

struct mousemotionevent {
  float_t x;
  float_t y;
};

struct mousebuttonevent {
  enum class type : uint32_t {
    down = SDL_EVENT_MOUSE_BUTTON_DOWN,
    up = SDL_EVENT_MOUSE_BUTTON_UP,
  };

  enum class button : uint8_t {
    left = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right = SDL_BUTTON_RIGHT,
  };

  type type;
  button button;
  float_t x;
  float_t y;
};

enum class joystickevent : int32_t {
  up = SDL_GAMEPAD_BUTTON_DPAD_UP,
  down = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
  left = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
  right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
  triangle = SDL_GAMEPAD_BUTTON_NORTH,
  circle = SDL_GAMEPAD_BUTTON_EAST,
  cross = SDL_GAMEPAD_BUTTON_SOUTH,
  square = SDL_GAMEPAD_BUTTON_WEST,
};

struct joystickaxisevent {
  enum class axis : int8_t {
    invalid = SDL_GAMEPAD_AXIS_INVALID,
    leftx = SDL_GAMEPAD_AXIS_LEFTX,
    lefty = SDL_GAMEPAD_AXIS_LEFTY,
    rightx = SDL_GAMEPAD_AXIS_RIGHTX,
    righty = SDL_GAMEPAD_AXIS_RIGHTY,
    triggerleft = SDL_GAMEPAD_AXIS_LEFT_TRIGGER,
    triggerright = SDL_GAMEPAD_AXIS_RIGHT_TRIGGER,
    max = SDL_GAMEPAD_AXIS_COUNT
  };

  axis kind;
  int16_t value;
};

struct mailevent {
  mailevent(uint64_t to, const std::string &body) noexcept
      : to(to), body(body) {}

  uint64_t to;
  std::string body;
};
}

struct collisionevent {
  collisionevent(uint64_t a, uint64_t b)
      : a(a), b(b) {}

  uint64_t a;
  uint64_t b;
};
