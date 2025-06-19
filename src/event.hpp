#pragma once

#include "common.hpp"

namespace input {
namespace event {
enum class player : uint8_t {
  one = 0,
  two
};

enum class type : uint32_t {
  collision = SDL_EVENT_USER + 1,
  mail,
  timer
};

namespace keyboard {
enum class key : int32_t {
  up = SDLK_UP,
  left = SDLK_LEFT,
  down = SDLK_DOWN,
  right = SDLK_RIGHT,
  space = SDLK_SPACE,

  backspace = SDLK_BACKSPACE,
  enter = SDLK_RETURN,
  escape = SDLK_ESCAPE
};
}

namespace mouse {
struct motion {
  float_t x;
  float_t y;
};

struct button {
  enum class type : uint32_t {
    down = SDL_EVENT_MOUSE_BUTTON_DOWN,
    up = SDL_EVENT_MOUSE_BUTTON_UP,
  };

  enum class which : uint8_t {
    left = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right = SDL_BUTTON_RIGHT,
  };

  type type;
  which button;
  float_t x;
  float_t y;
};
}

namespace gamepad {
enum class button : int32_t {
  up = SDL_GAMEPAD_BUTTON_DPAD_UP,
  down = SDL_GAMEPAD_BUTTON_DPAD_DOWN,
  left = SDL_GAMEPAD_BUTTON_DPAD_LEFT,
  right = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,
  north = SDL_GAMEPAD_BUTTON_NORTH,
  east = SDL_GAMEPAD_BUTTON_EAST,
  south = SDL_GAMEPAD_BUTTON_SOUTH,
  west = SDL_GAMEPAD_BUTTON_WEST,
};

struct motion {
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
}

struct mail final {
  mail(uint64_t to, const std::string& body)
      : to(to), body(body) {}

  uint64_t to;
  std::string body;
};

struct collision final {
  collision(uint64_t a, uint64_t b) noexcept
      : a(a), b(b) {}

  uint64_t a;
  uint64_t b;
};
}
}
