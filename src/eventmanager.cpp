#include "eventmanager.hpp"
#include "envelope.hpp"
#include <SDL3/SDL_gamepad.h>

using namespace input;
using namespace event;

eventmanager::eventmanager(std::shared_ptr<graphics::renderer> renderer)
    : _renderer(std::move(renderer)),
      _envelopepool(framework::envelopepool::instance()) {
  int32_t number;
  _joystickmapping.reserve(8);
  _mappingorder.reserve(8);
  std::unique_ptr<SDL_JoystickID[], decltype(&SDL_free)> joysticks(SDL_GetGamepads(&number), SDL_free);

  if (joysticks) {
    for (auto index = 0; index < number; ++index) {
      const auto gamepad_id = joysticks[static_cast<size_t>(index)];
      if (!SDL_IsGamepad(gamepad_id)) {
        continue;
      }

      if (const auto controller = SDL_OpenGamepad(gamepad_id)) {
        const auto jid = SDL_GetJoystickID(SDL_GetGamepadJoystick(controller));
        _controllers.emplace(jid, std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller));

        _mappingorder.push_back(jid);
        _joystickmapping[jid] = static_cast<uint8_t>(_mappingorder.size() - 1);
      }
    }
  }
}

void eventmanager::update(float_t delta) noexcept {
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
          const auto jid = SDL_GetJoystickID(SDL_GetGamepadJoystick(controller));

          _controllers[jid] = std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller);

          _mappingorder.push_back(jid);
          _joystickmapping[jid] = static_cast<uint8_t>(_mappingorder.size() - 1);
        }
      } break;

      case SDL_EVENT_GAMEPAD_REMOVED: {
        const auto it = _joystickmapping.find(event.cdevice.which);
          if (it == _joystickmapping.end()) {
            _controllers.erase(event.cdevice.which);
            break;
          }

          const auto index = it->second;
          const SDL_JoystickID rid = event.cdevice.which;
          const SDL_JoystickID lid = _mappingorder.back();

          _mappingorder[index] = lid;
          _joystickmapping[lid] = index;

          _mappingorder.pop_back();
          _joystickmapping.erase(rid);
          _controllers.erase(rid);
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const uint8_t slot = it->second;

        const gamepad::button e{event.gbutton.button};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_press(slot, e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const uint8_t slot = it->second;

        const gamepad::button e{event.gbutton.button};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_release(slot, e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const uint8_t slot = it->second;

        const auto axis = static_cast<gamepad::motion::axis>(event.gaxis.axis);
        const auto value = event.gaxis.value;
        const gamepad::motion e{axis, value};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_motion(slot, e);
        }
      } break;

      case static_cast<uint32_t>(type::collision): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);
        if (ptr) {
          const auto& payload = ptr->as_collision();
          const auto o = collision(payload.a, payload.b);
          for (const auto& receiver : _receivers) {
            receiver->on_collision(o);
          }

          _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
        }
      } break;

      case static_cast<uint32_t>(type::mail): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);

        if (ptr) {
          const auto& payload = ptr->as_mail();
          const auto o = mail(payload.to, payload.body);
          for (const auto& receiver : _receivers) {
            receiver->on_mail(o);
          }

          _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
        }
      } break;

      case static_cast<uint32_t>(type::timer): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);

        const auto& payload = ptr->as_timer();

        std::invoke(payload.fn);

        if (!payload.repeat) {
          _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
        }
      } break;

      default:
        break;
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
