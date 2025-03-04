#include "eventmanager.hpp"
#include "event.hpp"

using namespace input;

eventmanager::eventmanager() {
  const auto number = SDL_NumJoysticks();
  for (auto id = 0; id < number; ++id) {
    if (!SDL_IsGamepad(id)) {
      continue;
    }

    if (auto controller = SDL_OpenGamepad(id)) {
      _controllers.emplace(SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)), gamepad_ptr(controller));
    }
  }
}

void eventmanager::update(float_t delta) {
  UNUSED(delta);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT: {
      for (const auto &receiver : _receivers) {
        receiver->on_quit();
      }
    } break;

    case SDL_KEYDOWN: {
      const keyevent e{event.key.keysym.sym};

      for (const auto &receiver : _receivers) {
        receiver->on_keydown(e);
      }
    } break;

    case SDL_KEYUP: {
      const keyevent e{event.key.keysym.sym};

      for (const auto &receiver : _receivers) {
        receiver->on_keyup(e);
      }
    } break;

    case SDL_EVENT_MOUSE_MOTION: {
      const mousemotionevent e{event.motion.x, event.motion.y };

      for (const auto &receiver : _receivers) {
        receiver->on_mousemotion(e);
      }
    } break;

    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      const mousebuttonevent e{
          .type = mousebuttonevent::type::down,
          .button = static_cast<enum mousebuttonevent::button>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
      };

      for (const auto &receiver : _receivers) {
        receiver->on_mousebuttondown(e);
      }
      break;
    }

    case SDL_EVENT_MOUSE_BUTTON_UP: {
      const mousebuttonevent e{
          .type = mousebuttonevent::type::up,
          .button = static_cast<enum mousebuttonevent::button>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
      };

      for (const auto &receiver : _receivers) {
        receiver->on_mousebuttonup(e);
      }
    } break;

    case SDL_EVENT_GAMEPAD_ADDED: {
      if (!SDL_IsGamepad(event.cdevice.which)) {
        break;
      }

      if (auto controller = SDL_OpenGamepad(event.cdevice.which)) {
        const auto joystick = SDL_GetGamepadJoystick(controller);
        const auto id = SDL_GetJoystickID(joystick);
        _controllers[id] = std::unique_ptr<SDL_GameController, SDL_Deleter>(controller)
      }
    } break;

    case SDL_EVENT_GAMEPAD_REMOVED:
      _controllers.erase(event.cdevice.which);
      break;

    case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
      const joystickevent e{event.gbutton.button};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttondown(event.gbutton.which, e);
      }
    } break;

    case SDL_EVENT_GAMEPAD_BUTTON_UP: {
      const joystickevent e{event.gbutton.button};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttonup(event.gbutton.which, e);
      }
    } break;

    case SDL_EVENT_GAMEPAD_TOUCHPAD_MOTION: {
      const auto who = event.gaxis.which;
      const auto axis = static_cast<input::joystickaxisevent::axis>(event.gaxis.axis);
      const auto value = event.gaxis.value;
      const joystickaxisevent e{axis, value};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickaxismotion(who, e);
      }
    } break;

    case input::eventtype::collision: {
      auto *ptr = static_cast<framework::collision *>(event.user.data1);
      if (ptr) {
        for (const auto &receiver : _receivers) {
          receiver->on_collision(collisionevent(ptr->a, ptr->b));
        }
        delete ptr;
      }
    } break;

    case input::eventtype::mail: {
      auto *ptr = static_cast<framework::mail *>(event.user.data1);
      if (ptr) {
        for (const auto &receiver : _receivers) {
          receiver->on_mail(mailevent(ptr->to, ptr->body));
        }

        delete ptr;
      }
    } break;

    case input::eventtype::timer: {
      const auto *fn = static_cast<std::function<void()> *>(event.user.data1);
      const auto *repeat = static_cast<bool *>(event.user.data2);
      if (fn) {
        (*fn)();

        if (repeat && !(*repeat)) {
          delete fn;
          delete repeat;
        }
      }
    } break;

    default:
      break;
    }
  }
}

void eventmanager::add_receiver(std::shared_ptr<eventreceiver> receiver) noexcept {
  _receivers.emplace_back(receiver);
}

void eventmanager::remove_receiver(std::shared_ptr<eventreceiver> receiver) noexcept {
  _receivers.erase(std::remove(_receivers.begin(), _receivers.end(), receiver), _receivers.end());
}
