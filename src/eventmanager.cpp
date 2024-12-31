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

  static constexpr std::array<std::pair<Uint8, SDL_Keycode>, 6> mapping = {{
      {SDL_CONTROLLER_BUTTON_DPAD_UP, SDLK_UP},
      {SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDLK_LEFT},
      {SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDLK_DOWN},
      {SDL_CONTROLLER_BUTTON_DPAD_RIGHT, SDLK_RIGHT},
      {SDL_CONTROLLER_BUTTON_A, SDLK_SPACE},
      {SDL_CONTROLLER_BUTTON_B, SDLK_SPACE},
  }};

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

    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP: {
      const auto it = std::find_if(mapping.begin(), mapping.end(), [&event](const auto &pair) { return pair.first == event.cbutton.button; });

      if (it != mapping.end()) {
        const auto action = (event.type == SDL_CONTROLLERBUTTONDOWN) ? &eventreceiver::on_keydown : &eventreceiver::on_keyup;
        for (const auto &receiver : _receivers) {
          (receiver.get()->*action)(keyevent(it->second));
        }
      }
    } break;

    case SDL_CONTROLLERAXISMOTION: {
      static constexpr auto threshold = 8000;
      static constexpr auto deadzone = 4000;

      const auto axis = event.caxis.axis;
      const auto value = event.caxis.value;
      const auto process = [&](SDL_Keycode negative, SDL_Keycode positive) {
        if (value < -threshold) {
          for (const auto &receiver : _receivers) {
            receiver->on_keydown(keyevent(negative));
          }
        } else if (value > threshold) {
          for (const auto &receiver : _receivers) {
            receiver->on_keydown(keyevent(positive));
          }
        } else if (std::abs(value) < deadzone) {
          for (const auto &receiver : _receivers) {
            receiver->on_keyup(keyevent(negative));
            receiver->on_keyup(keyevent(positive));
          }
        }
      };

      switch (axis) {
      case SDL_CONTROLLER_AXIS_LEFTY:
        process(SDLK_UP, SDLK_DOWN);
        break;

      case SDL_CONTROLLER_AXIS_LEFTX:
        process(SDLK_LEFT, SDLK_RIGHT);
        break;

      default:
        break;
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
      auto *ptr = static_cast<std::function<void()> *>(event.user.data1);
      if (ptr) {
        (*ptr)();
        delete ptr;
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
