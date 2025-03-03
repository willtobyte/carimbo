#pragma once

#include "common.hpp"

namespace input {
enum player : uint8_t {
  one = 0,
  two
};

enum eventtype : uint32_t {
  collision = SDL_USEREVENT + 1,
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
  int32_t x;
  int32_t y;

  constexpr mousemotionevent(int32_t x, int32_t y)
      : x(x), y(y) {}
};

struct mousebuttonevent {
  enum class Type : uint32_t {
    down = SDL_MOUSEBUTTONDOWN,
    up = SDL_MOUSEBUTTONUP,
  };

  enum class Button : uint8_t {
    left = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right = SDL_BUTTON_RIGHT,
  };

  Type type;
  Button button;
  int32_t x;
  int32_t y;

  constexpr mousebuttonevent(Type type, Button button, int32_t x, int32_t y)
      : type(type), button(button), x(x), y(y) {}
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

struct joystickaxisevent {
  enum class axis : int8_t {
    invalid = SDL_CONTROLLER_AXIS_INVALID,
    leftx = SDL_CONTROLLER_AXIS_LEFTX,
    lefty = SDL_CONTROLLER_AXIS_LEFTY,
    rightx = SDL_CONTROLLER_AXIS_RIGHTX,
    righty = SDL_CONTROLLER_AXIS_RIGHTY,
    triggerleft = SDL_CONTROLLER_AXIS_TRIGGERLEFT,
    triggerright = SDL_CONTROLLER_AXIS_TRIGGERRIGHT,
    max = SDL_CONTROLLER_AXIS_MAX
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
