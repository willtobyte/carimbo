#pragma once

#include "common.hpp"

namespace event {
namespace keyboard {
enum key : int32_t {
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
  float x;
  float y;
};

struct button {
  enum type : uint32_t {
    down = SDL_EVENT_MOUSE_BUTTON_DOWN,
    up = SDL_EVENT_MOUSE_BUTTON_UP,
  };

  enum which : uint8_t {
    left = SDL_BUTTON_LEFT,
    middle = SDL_BUTTON_MIDDLE,
    right = SDL_BUTTON_RIGHT,
  };

  type type;
  which button;
  float x;
  float y;
};
}
}
