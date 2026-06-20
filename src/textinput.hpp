#pragma once

#include "common.hpp"

#include "noncopyable.hpp"

class textinput final : private noncopyable {
public:
  textinput();

  ~textinput() override;

  void on(sol::protected_function fn);

  void off();

private:
  static bool dispatch(void* userdata, SDL_Event* event);

  sol::protected_function _callback;
};
