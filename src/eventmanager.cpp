#include "eventmanager.hpp"

using namespace input;
using namespace event;

eventmanager::eventmanager(std::shared_ptr<graphics::renderer> renderer)
  : _renderer(std::move(renderer)) {
  int32_t number;
  std::unique_ptr<SDL_JoystickID[], decltype(&SDL_free)> joysticks(SDL_GetGamepads(&number), SDL_free);
  if (!joysticks) {
    return;
  }

  for (auto index = 0; index < number; ++index) {
    const auto gamepad_id = joysticks[static_cast<size_t>(index)];
    if (!SDL_IsGamepad(gamepad_id)) {
      continue;
    }

    if (const auto controller = SDL_OpenGamepad(gamepad_id)) {
      _controllers.emplace(
        SDL_GetJoystickID(SDL_GetGamepadJoystick(controller)),
        std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller)
      );
    }
  }
}

void eventmanager::update(float_t delta) {
  UNUSED(delta);

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    SDL_ConvertEventToRenderCoordinates(*_renderer, &event);

    switch (event.type) {
      case SDL_EVENT_QUIT: {
        for (const auto& receiver : _receivers) {
          receiver->on_quit();
        }
      } break;

      case SDL_EVENT_KEY_DOWN: {
        const keyboard::key e{static_cast<keyboard::key>(event.key.key)};
        for (const auto& receiver : _receivers) {
          receiver->on_key_press(e);
        }
      } break;

      case SDL_EVENT_KEY_UP: {
        const keyboard::key e{static_cast<keyboard::key>(event.key.key)};
        for (const auto& receiver : _receivers) {
          receiver->on_key_release(e);
        }
      } break;

      case SDL_EVENT_TEXT_INPUT: {
        const std::string t{event.text.text};

        for (const auto& receiver : _receivers) {
          receiver->on_text(t);
        }
      } break;

      case SDL_EVENT_MOUSE_MOTION: {
        const mouse::motion e{event.motion.x, event.motion.y};
        for (const auto& receiver : _receivers) {
          receiver->on_mouse_motion(e);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        const mouse::button e{
          .type = mouse::button::type::down,
          .button = static_cast<mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };
        for (const auto& receiver : _receivers) {
          receiver->on_mouse_press(e);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP: {
        const mouse::button e{
          .type = mouse::button::type::up,
          .button = static_cast<mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };
        for (const auto& receiver : _receivers) {
          receiver->on_mouse_release(e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_ADDED: {
        if (!SDL_IsGamepad(event.cdevice.which)) {
          break;
        }

        if (auto controller = SDL_OpenGamepad(event.cdevice.which)) {
          const auto joystick = SDL_GetGamepadJoystick(controller);
          const auto id = SDL_GetJoystickID(joystick);
          _controllers[id] = std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller);
        }
      } break;

      case SDL_EVENT_GAMEPAD_REMOVED:
        _controllers.erase(event.cdevice.which);
        break;

      case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        const gamepad::button e{event.gbutton.button};
        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_press(static_cast<uint8_t>(event.gbutton.which), e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const gamepad::button e{event.gbutton.button};
        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_release(static_cast<uint8_t>(event.gbutton.which), e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const auto who = event.gaxis.which;
        const auto axis = static_cast<gamepad::motion::axis>(event.gaxis.axis);
        const auto value = event.gaxis.value;
        const gamepad::motion e{axis, value};
        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_motion(static_cast<uint8_t>(who), e);
        }
      } break;

      case static_cast<uint32_t>(type::collision): {
        auto* ptr = static_cast<framework::collision*>(event.user.data1);
        if (ptr) {
          for (const auto& receiver : _receivers) {
            receiver->on_collision(collision(ptr->a, ptr->b));
          }
          delete ptr;
        }
      } break;

      case static_cast<uint32_t>(type::mail): {
        auto* ptr = static_cast<framework::mail*>(event.user.data1);
        if (ptr) {
          for (const auto& receiver : _receivers) {
            receiver->on_mail(mail(ptr->to, ptr->body));
          }
          delete ptr;
        }
      } break;

      case static_cast<uint32_t>(type::timer): {
        const auto* repeat = static_cast<bool*>(event.user.data2);
        if (!repeat) {
          break;
        }
        if (*repeat) {
          const auto* fn = static_cast<std::function<void()>*>(
            event.user.data1
          );
          if (fn) {
            (*fn)();
          }
          break;
        }
        std::unique_ptr<std::function<void()>> fn{
          static_cast<std::function<void()>*>(event.user.data1)
        };
        if (fn) {
          (*fn)();
        }
      } break;

      default:
        return;
    }
  }
}

void eventmanager::add_receiver(std::shared_ptr<eventreceiver> receiver) {
  _receivers.emplace_back(receiver);
}

void eventmanager::remove_receiver(std::shared_ptr<eventreceiver> receiver) {
  _receivers.erase(
    std::remove(_receivers.begin(), _receivers.end(), receiver),
    _receivers.end()
  );
}
