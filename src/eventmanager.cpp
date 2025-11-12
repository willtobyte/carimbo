#include "eventmanager.hpp"
#include "envelope.hpp"
#include <SDL3/SDL_gamepad.h>
#include <SDL3/SDL_keycode.h>

using namespace input;
using namespace event;

eventmanager::eventmanager(std::shared_ptr<graphics::renderer> renderer)
    : _renderer(std::move(renderer)) {
  int32_t number;

  constexpr auto rc = 8uz;
  _joystickmapping.reserve(rc);
  _joystickgorder.reserve(rc);

  std::unique_ptr<SDL_JoystickID[], SDL_Deleter> joysticks(SDL_GetGamepads(&number));

  if (joysticks) {
    for (auto index = 0; index < number; ++index) {
      const auto gamepad_id = joysticks[static_cast<size_t>(index)];
      if (!SDL_IsGamepad(gamepad_id)) {
        continue;
      }

      if (const auto controller = SDL_OpenGamepad(gamepad_id)) {
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
  _receivers.erase(
    std::remove_if(
      _receivers.begin(),
      _receivers.end(),
      [](const auto& weak) {
        return weak.expired();
      }),
    _receivers.end());

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    SDL_ConvertEventToRenderCoordinates(*_renderer, &event);

    switch (event.type) {
      case SDL_EVENT_QUIT: {
        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_quit();
          }
        }
      } break;

      case SDL_EVENT_KEY_DOWN: {
        const keyboard::key e{static_cast<keyboard::key>(event.key.key)};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_key_press(e);
          }
        }
      } break;

      case SDL_EVENT_KEY_UP: {
        switch (event.key.key) {
          case SDLK_F11: {
            auto* const window = static_cast<SDL_Window*>(*_renderer);
            const auto fullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
            SDL_SetWindowFullscreen(window, !fullscreen);
          } break;
          default:
            break;
        }

#ifndef NDEBUG
        case SDLK_D: {
          if (event.key.mod & SDL_KMOD_CTRL) {
            for (const auto& weak : _receivers) {
              if (auto receiver = weak.lock(); receiver) {
                receiver->on_debug();
              }
            }
            break;
          }
        }
#endif

        const keyboard::key e{static_cast<keyboard::key>(event.key.key)};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_key_release(e);
          }
        }
      } break;

      case SDL_EVENT_TEXT_INPUT: {
        const std::string t{event.text.text};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_text(t);
          }
        }
      } break;

      case SDL_EVENT_MOUSE_MOTION: {
        const mouse::motion e{event.motion.x, event.motion.y};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_mouse_motion(e);
          }
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        const mouse::button e{
          .type = mouse::button::type::down,
          .button = static_cast<mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_mouse_press(e);
          }
        }
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP: {
        const mouse::button e{
          .type = mouse::button::type::up,
          .button = static_cast<mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_mouse_release(e);
          }
        }
      } break;

      case SDL_EVENT_GAMEPAD_ADDED: {
        if (!SDL_IsGamepad(event.cdevice.which)) {
          break;
        }

        if (auto controller = SDL_OpenGamepad(event.cdevice.which)) {
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
          const SDL_JoystickID rid = event.cdevice.which;
          const SDL_JoystickID lid = _joystickgorder.back();

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

        const gamepad::button e{event.gbutton.button};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_gamepad_press(slot, e);
          }
        }
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const gamepad::button e{event.gbutton.button};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_gamepad_release(slot, e);
          }
        }
      } break;

      case SDL_EVENT_GAMEPAD_AXIS_MOTION: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const auto axis = static_cast<gamepad::motion::axis>(event.gaxis.axis);
        const auto value = event.gaxis.value;
        const gamepad::motion e{axis, value};

        for (const auto& weak : _receivers) {
          if (auto receiver = weak.lock(); receiver) {
            receiver->on_gamepad_motion(slot, e);
          }
        }
      } break;

      case static_cast<uint32_t>(type::collision): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);
        if (ptr) {
          if (const auto* payload = ptr->try_collision(); payload) {
            const auto o = collision(payload->a, payload->b);
            for (const auto& weak : _receivers) {
              if (auto receiver = weak.lock(); receiver) {
                receiver->on_collision(o);
              }
            }
          }

          _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
        }
      } break;

      case static_cast<uint32_t>(type::mail): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);

        if (ptr) {
          if (const auto* payload = ptr->try_mail(); payload) {
            const auto o = mail(payload->to, payload->body);
            for (const auto& weak : _receivers) {
              if (auto receiver = weak.lock(); receiver) {
                receiver->on_mail(o);
              }
            }
          }

          _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
        }
      } break;

      case static_cast<uint32_t>(type::timer): {
        auto* ptr = static_cast<framework::envelope*>(event.user.data1);

        if (const auto* payload = ptr->try_timer(); payload) {
          const auto release = !payload->repeat;
          const auto fn = payload->fn;

          if (fn) {
            fn();
          }

          if (release) {
            _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
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

  std::owner_less<void> cmp;
  for (auto i = _receivers.size(); i-- > 0; ) {
    auto& weak = _receivers[i];
    if (weak.expired() || (!cmp(weak, receiver) && !cmp(receiver, weak))) {
      _receivers[i] = std::move(_receivers.back());
      _receivers.pop_back();
    }
  }
}

void eventmanager::purge(uint32_t begin_event, uint32_t end_event) {
  if (end_event == 0) end_event = begin_event;

  SDL_Event event;
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, begin_event, end_event) > 0) {
    if (!event.user.data1) {
      continue;
    }

    auto* ptr = static_cast<framework::envelope*>(event.user.data1);

    _envelopepool->release(std::unique_ptr<framework::envelope>(ptr));
  }
}
