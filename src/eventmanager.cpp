#include "eventmanager.hpp"
#include "event.hpp"

using namespace input;

eventmanager::eventmanager() {
  const auto count = SDL_NumJoysticks();
  for (const auto id : std::views::iota(0, count)) {
    if (!SDL_IsGameController(id)) {
      continue;
    }

    if (auto *controller = SDL_GameControllerOpen(id)) {
      const auto joystick = SDL_GameControllerGetJoystick(controller);
      const auto instanceId = SDL_JoystickInstanceID(joystick);
      _controllers.emplace(instanceId, std::unique_ptr<SDL_GameController, SDL_Deleter>(controller));
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

    case SDL_MOUSEMOTION: {
      const mousemotionevent e{
          event.motion.x, event.motion.y
      };

      for (const auto &receiver : _receivers) {
        receiver->on_mousemotion(e);
      }
    } break;

    case SDL_MOUSEBUTTONDOWN: {
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
    case SDL_MOUSEBUTTONUP: {
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

    case SDL_CONTROLLERDEVICEADDED: {
      if (!SDL_IsGameController(event.cdevice.which))
        break;

      auto controller = SDL_GameControllerOpen(event.cdevice.which);
      if (!controller)
        break;

      const auto joystick = SDL_GameControllerGetJoystick(controller);
      const auto id = SDL_JoystickInstanceID(joystick);
      if (_controllers.find(id) != _controllers.end()) {
        SDL_GameControllerClose(controller);
        break;
      }

      _controllers.emplace(id, gamecontroller_ptr(controller));
    } break;

    case SDL_CONTROLLERDEVICEREMOVED:
      _controllers.erase(event.cdevice.which);
      break;

    case SDL_CONTROLLERBUTTONDOWN: {
      const joystickevent e{event.cbutton.button};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttondown(event.cbutton.which, e);
      }
    } break;

    case SDL_CONTROLLERBUTTONUP: {
      const joystickevent e{event.cbutton.button};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttonup(event.cbutton.which, e);
      }
    } break;

    case SDL_CONTROLLERAXISMOTION: {
      const auto who = event.caxis.which;
      const auto axis = static_cast<input::joystickaxisevent::axis>(event.caxis.axis);
      const auto value = event.caxis.value;
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
  _receivers.emplace_back(std::move(receiver));
}

void eventmanager::remove_receiver(std::shared_ptr<eventreceiver> receiver) noexcept {
  _receivers.erase(std::remove(_receivers.begin(), _receivers.end(), receiver), _receivers.end());
}
