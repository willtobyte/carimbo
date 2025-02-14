#include "eventmanager.hpp"
#include "event.hpp"

using namespace input;

eventmanager::eventmanager() {
  const auto number = SDL_NumJoysticks();
  for (auto id = 0; id < number; ++id) {
    if (SDL_IsGameController(id)) {
      if (auto controller = SDL_GameControllerOpen(id)) {
        _controllers.emplace(SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(controller)), gamecontroller_ptr(controller));
      }
    }
  }
}

void eventmanager::update(float_t delta) {
  UNUSED(delta);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      for (const auto &receiver : _receivers) {
        receiver->on_quit();
      }
      break;

    case SDL_KEYDOWN:
      for (const auto &receiver : _receivers) {
        receiver->on_keydown(keyevent(event.key.keysym.sym));
      }
      break;

    case SDL_KEYUP:
      for (const auto &receiver : _receivers) {
        receiver->on_keyup(keyevent(event.key.keysym.sym));
      }
      break;

    case SDL_CONTROLLERDEVICEADDED:
      if (SDL_IsGameController(event.cdevice.which)) {
        if (auto controller = SDL_GameControllerOpen(event.cdevice.which)) {
          const auto joystick = SDL_GameControllerGetJoystick(controller);
          const auto id = SDL_JoystickInstanceID(joystick);
          _controllers[id] = gamecontroller_ptr(controller);
        }
      }
      break;

    case SDL_CONTROLLERDEVICEREMOVED:
      _controllers.erase(event.cdevice.which);
      break;

    case SDL_CONTROLLERBUTTONDOWN: {
      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttondown(event.cbutton.which, joystickevent(event.cbutton.button));
      }
    } break;

    case SDL_CONTROLLERBUTTONUP: {
      for (const auto &receiver : _receivers) {
        receiver->on_joystickbuttonup(event.cbutton.which, joystickevent(event.cbutton.button));
      }
    } break;

    case SDL_CONTROLLERAXISMOTION: {
      const auto who = event.caxis.which;
      const auto axis = static_cast<input::joystickaxisevent::axis>(event.caxis.axis);
      const auto value = event.caxis.value;

      joystickaxisevent event = {axis, value};

      for (const auto &receiver : _receivers) {
        receiver->on_joystickaxismotion(who, event);
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
  _receivers.remove(receiver);
}
