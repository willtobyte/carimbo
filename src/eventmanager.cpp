#include "eventmanager.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"

eventmanager::eventmanager() {
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

        auto [it, inserted] = _controllers.try_emplace(jid);
        if (!inserted) {
          SDL_CloseGamepad(controller);
          break;
        }

        it->second = std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller);

        _joystickgorder.emplace_back(jid);
        _joystickmapping[jid] = static_cast<uint8_t>(_joystickgorder.size() - 1);

        std::println("[eventmanager] gamepad connected {}", jid);
      }
    }
  }
}

void eventmanager::update(float delta) {
  SDL_Event event;

  const auto prune = [&]() {
    _receivers.erase(
      std::remove_if(
        _receivers.begin(),
        _receivers.end(),
        [](const auto& receiver) {
          return receiver.expired();
        }
      ),
      _receivers.end()
    );
  };

  const auto dispatch = [&](auto&& call) {
    auto dirty = false;
    for (auto it = _receivers.begin(); it != _receivers.end(); ++it) {
      if (auto receiver = it->lock()) {
        call(receiver);
      } else {
        dirty = true;
      }
    }

    if (dirty) {
      prune();
    }
  };

  while (SDL_PollEvent(&event)) {
    SDL_ConvertEventToRenderCoordinates(renderer, &event);

    switch (event.type) {
      case SDL_EVENT_QUIT: {
        dispatch([](const auto& receiver) {
          receiver->on_quit();
        });
      } break;

      case SDL_EVENT_KEY_DOWN: {
        const event::keyboard::key e{static_cast<event::keyboard::key>(event.key.key)};

        dispatch([&](const auto& receiver) {
          receiver->on_key_press(e);
        });
      } break;

      case SDL_EVENT_KEY_UP: {
        switch (event.key.key) {
          case SDLK_F11: {
            auto* const window = SDL_GetRenderWindow(renderer);
            const auto fullscreen = (SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN) != 0;
            SDL_SetWindowFullscreen(window, !fullscreen);
          } break;
#ifndef NDEBUG
          case SDLK_D: {
            if (event.key.mod & SDL_KMOD_CTRL) {
              dispatch([](const auto& receiver) {
                receiver->on_debug();
              });
            }
          } break;
#endif
          default:
            break;
        }

        const event::keyboard::key e{static_cast<event::keyboard::key>(event.key.key)};

        dispatch([&](const auto& receiver) {
          receiver->on_key_release(e);
        });
      } break;

      case SDL_EVENT_TEXT_INPUT: {
        dispatch([&](const auto& receiver) {
          receiver->on_text(event.text.text);
        });
      } break;

      case SDL_EVENT_MOUSE_MOTION: {
        const event::mouse::motion e{event.motion.x, event.motion.y};

        dispatch([&](const auto& receiver) {
          receiver->on_mouse_motion(e);
        });
      } break;

      case SDL_EVENT_MOUSE_BUTTON_DOWN: {
        const event::mouse::button e{
          .type = event::mouse::button::type::down,
          .button = static_cast<event::mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };

        dispatch([&](const auto& receiver) {
          receiver->on_mouse_press(e);
        });
      } break;

      case SDL_EVENT_MOUSE_BUTTON_UP: {
        const event::mouse::button e{
          .type = event::mouse::button::type::up,
          .button = static_cast<event::mouse::button::which>(event.button.button),
          .x = event.button.x,
          .y = event.button.y
        };

        dispatch([&](const auto& receiver) {
          receiver->on_mouse_release(e);
        });
      } break;

      case SDL_EVENT_GAMEPAD_ADDED: {
        if (!SDL_IsGamepad(event.cdevice.which)) {
          break;
        }

        if (auto* const controller = SDL_OpenGamepad(event.cdevice.which)) {
          const auto jid = SDL_GetJoystickID(SDL_GetGamepadJoystick(controller));

          auto [it, inserted] = _controllers.try_emplace(jid);
          if (!inserted) {
            SDL_CloseGamepad(controller);
            break;
          }

          it->second = std::unique_ptr<SDL_Gamepad, SDL_Deleter>(controller);

          _joystickgorder.emplace_back(jid);
          _joystickmapping[jid] = static_cast<uint8_t>(_joystickgorder.size() - 1);

          std::println("[eventmanager] gamepad connected {}", jid);
        }
      } break;

      case SDL_EVENT_GAMEPAD_REMOVED: {
        const auto rid = event.cdevice.which;
        const auto it = _joystickmapping.find(rid);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto index = it->second;

        if (index != _joystickgorder.size() - 1) {
          const auto lid = _joystickgorder.back();
          _joystickgorder[index] = lid;
          _joystickmapping[lid] = index;
        }

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

        dispatch([&](const auto& receiver) {
          receiver->on_gamepad_press(slot, e);
        });
      } break;

      case SDL_EVENT_GAMEPAD_BUTTON_UP: {
        const auto it = _joystickmapping.find(event.gbutton.which);
        if (it == _joystickmapping.end()) {
          break;
        }

        const auto slot = it->second;

        const event::gamepad::button e{event.gbutton.button};

        dispatch([&](const auto& receiver) {
          receiver->on_gamepad_release(slot, e);
        });
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

        dispatch([&](const auto& receiver) {
          receiver->on_gamepad_motion(slot, e);
        });
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

  _receivers.erase(
    std::remove_if(
      _receivers.begin(),
      _receivers.end(),
      [&](const auto& candidate) {
        if (auto locked = candidate.lock()) {
          return locked == receiver;
        }

        return true;
      }
    ),
    _receivers.end()
  );
}

void eventmanager::flush(uint32_t begin_event, uint32_t end_event) {
  if (end_event == 0) [[unlikely]] {
    end_event = begin_event;
  }

  SDL_Event event;
  while (SDL_PeepEvents(&event, 1, SDL_GETEVENT, begin_event, end_event) > 0) {
  }
}
