#include "textinput.hpp"

textinput::textinput() {
  SDL_AddEventWatch(&dispatch, this);
}

textinput::~textinput() {
  SDL_RemoveEventWatch(&dispatch, this);
}

void textinput::on(sol::protected_function fn) {
  _callback = std::move(fn);

  const auto properties = SDL_CreateProperties();
  SDL_SetBooleanProperty(properties, SDL_PROP_TEXTINPUT_AUTOCORRECT_BOOLEAN, false);
  SDL_StartTextInputWithProperties(SDL_GetRenderWindow(renderer), properties);
  SDL_DestroyProperties(properties);
}

void textinput::off() {
  _callback = sol::protected_function();

  SDL_StopTextInput(SDL_GetRenderWindow(renderer));
}

bool textinput::dispatch(void* userdata, SDL_Event* event) {
  if (event->type != SDL_EVENT_TEXT_INPUT) [[likely]] {
    return true;
  }

  auto* self = static_cast<textinput*>(userdata);
  if (!self->_callback.valid()) [[unlikely]] {
    return true;
  }

  const auto result = self->_callback(std::string_view{event->text.text});
  if (!result.valid()) [[unlikely]] {
    const sol::error err = result;
    std::println(stderr, "{}", err.what());
  }

  return true;
}
