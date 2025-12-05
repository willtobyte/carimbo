#include "eventmanager.hpp"

#include "envelope.hpp"
#include "event.hpp"
#include "eventreceiver.hpp"
#include "renderer.hpp"

#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_keycode.h>

eventmanager::eventmanager(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {
  _joystickmapping.reserve(8);
  _joystickgorder.reserve(8);

  _receivers.reserve(32);
  _controllers.reserve(8);

  auto number = 0;
  if (const auto gamepads = std::unique_ptr<uint32_t[], SDL_Deleter>(SDL_GetGamepads(&number))) {
    for (auto index = 0; index < number; ++index) {
      const auto gid = gamepads[static_cast<size_t>(index)];
      if (!SDL_IsGamepad(gid)) {
        continue;
      }

      if (auto* const controller = SDL_OpenGamepad(gid)) {
        const auto jid = SDL_GetJoystickID(SDL_GetGamepadJoystick(controller));

        if (_controllers.contains(jid)) {
          SDL_CloseGamepad(controller);
          break;
        }

        _controllers.emplace(jid, std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller));

        _joystickgorder.emplace_back(jid);
        _joystickmapping[jid] = static_cast<uint8_t>(_joystickgorder.size() - 1);

        std::println("[eventmanager] gamepad connected {}", jid);
      }
    }
  }
}

void eventmanager::update(float delta) {
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
        const event::keyboard::key e{static_cast<event::keyboard::key>(event.key.key)};

        for (const auto& receiver : _receivers) {
          receiver->on_key_press(e);
        }
      } break;

      case SDL_EVENT_KEY_UP: {
        switch (event.key.key) {
          case SDLK_F11: {
            auto* const window = static_cast<SDL_Window*>(*_renderer);
            const auto fullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
            SDL_SetWindowFullscreen(window, !fullscreen);
          } break;
#ifndef NDEBUG
          case SDLK_D: {
            if (event.key.mod & SDL_KMOD_CTRL) {
              for (const auto& receiver : _receivers) {
                receiver->on_debug();
              }
            }
          } break;
#endif
          default:
            break;
        }

        const event::keyboard::key e{static_cast<event::keyboard::key>(event.key.key)};

        for (const auto& receiver : _receivers) {
          receiver->on_key_release(e);
        }
      } break;

      case SDL_EVENT_TEXT_INPUT: {
        for (const auto& receiver : _receivers) {
          receiver->on_text(event.text.text);
        }
      } break;

      case SDL_EVENT_MOUSE_MOTION: {
        const event::mouse::motion e{event.motion.x, event.motion.y};

        for (const auto& receiver : _receivers) {
          receiver->on_mouse_motion(e);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        const event::mouse::button e{
          .type = event::mouse::button::type::down,
          .button = static_cast<event::mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };

        for (const auto& receiver : _receivers) {
          receiver->on_mouse_press(e);
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP: {
        const event::mouse::button e{
          .type = event::mouse::button::type::up,
          .button = static_cast<event::mouse::button::which>(event.button.button),
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

        if (auto* const controller = SDL_OpenGamepad(event.cdevice.which)) {
          const auto jid = SDL_GetJoystickID(SDL_GetGamepadJoystick(controller));

          if (_controllers.contains(jid)) {
            SDL_CloseGamepad(controller);
            break;
          }

          _controllers[jid] = std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller);

          _joystickgorder.emplace_back(jid);
          _joystickmapping[jid] = static_cast<uint8_t>(_joystickgorder.size() - 1);

          std::println("[eventmanager] gamepad connected {}", jid);
        }
      } break;

      case SDL_EVENT_GAMEPAD_REMOVED: {
        const auto it = _joystickmapping.find(event.cdevice.which);
          if (it == _joystickmapping.end()) {
            _controllers.erase(event.cdevice.which);
            break;
          }

          const auto index = it->second;
          const auto rid = event.cdevice.which;
          const auto lid = _joystickgorder.back();

          _joystickgorder[index] = lid;
          _joystickmapping[lid] = index;

          _joystickgorder.pop_back();
          _joystickmapping.erase(rid);
          _controllers.erase(rid);

          std::println("[eventmanager] gamepad disconnected {}", rid);
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_DOWN: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const event::gamepad::button e{event.gbutton.button};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_press(slot, e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const event::gamepad::button e{event.gbutton.button};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_release(slot, e);
        }
      } break;

      case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const auto axis = static_cast<event::gamepad::motion::axis>(event.gaxis.axis);
        const auto value = event.gaxis.value;
        const event::gamepad::motion e{axis, value};

        for (const auto& receiver : _receivers) {
          receiver->on_gamepad_motion(slot, e);
        }
      } break;

      case static_cast<uint32_t>(event::type::mail): {
        auto* ptr = static_cast<envelope*>(event.user.data1);

        if (ptr) {
          if (const auto* payload = ptr->try_mail(); payload) {
            const auto o = event::mail(payload->to, payload->body);
            for (const auto& receiver : _receivers) {
              receiver->on_mail(o);
            }
          }

          _envelopepool->release(ptr);
        }
      } break;

      case static_cast<uint32_t>(event::type::timer): {
        auto* ptr = static_cast<envelope*>(event.user.data1);

        if (ptr) {
          if (const auto* payload = ptr->try_timer(); payload) {
            const auto fn = payload->fn;
            const auto repeat = payload->repeat;

            if (!repeat) {
              _envelopepool->release(ptr);
            }

            if (fn) {
              fn();
            }
          }
        }
      } break;

      default:
        break;
    }
  }
}

void eventmanager::add_receiver(const std::shared_ptr<eventreceiver>& receiver) {
  if (!receiver) [[unlikely]] {
    return;
  }

  _receivers.emplace_back(receiver);
}

void eventmanager::remove_receiver(const std::shared_ptr<eventreceiver>& receiver) {
  if (!receiver) [[unlikely]] {
    return;
  }

  _receivers.erase(std::remove(_receivers.begin(), _receivers.end(), receiver), _receivers.end());
}

void eventmanager::flush(uint32_t begin_event, uint32_t end_event) {
  if (end_event == 0) [[unlikely]] {
    end_event = begin_event;
  }

  SDL_Event event;
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, begin_event, end_event) > 0) {
    if (!event.user.data1) [[likely]] {
      continue;
    }

    auto* ptr = static_cast<envelope*>(const_cast<void*>(event.user.data1));
    _envelopepool->release(ptr);
  }
}
