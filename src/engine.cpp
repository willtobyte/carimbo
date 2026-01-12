#include "engine.hpp"

#include "audiodevice.hpp"
#include "constant.hpp"
#include "eventmanager.hpp"
#include "loopable.hpp"
#include "renderer.hpp"
#include "scenemanager.hpp"
#include "window.hpp"

std::shared_ptr<audiodevice> engine::audiodevice() const noexcept {
  return _audiodevice;
}

std::shared_ptr<eventmanager> engine::eventmanager() const noexcept {
  return _eventmanager;
}

std::shared_ptr<scenemanager> engine::scenemanager() const noexcept {
  return _scenemanager;
}

std::shared_ptr<window> engine::window() const noexcept {
  return _window;
}

std::shared_ptr<renderer> engine::renderer() const noexcept {
  return _renderer;
}

std::shared_ptr<::overlay> engine::overlay() const noexcept {
  return _overlay;
}

std::shared_ptr<::canvas> engine::canvas() const noexcept {
  return _canvas;
}

void engine::set_audiodevice(std::shared_ptr<::audiodevice> ptr) noexcept {
  _audiodevice = std::move(ptr);
}

void engine::set_eventmanager(std::shared_ptr<::eventmanager> ptr) noexcept {
  _eventmanager = std::move(ptr);
}

void engine::set_scenemanager(std::shared_ptr<::scenemanager> ptr) noexcept {
  _scenemanager = std::move(ptr);
}

void engine::set_window(std::shared_ptr<::window> ptr) noexcept {
  _window = std::move(ptr);
}

void engine::set_renderer(std::shared_ptr<::renderer> ptr) noexcept {
  _renderer = std::move(ptr);

  _canvas = std::make_shared<::canvas>(_renderer);
}

void engine::set_overlay(std::shared_ptr<::overlay> ptr) noexcept {
  _overlay = std::move(ptr);
}

void engine::set_ticks(uint8_t ticks) noexcept {
  _ticks = ticks;
  _tick_interval = ticks > 0 ? 1.0f / static_cast<float>(ticks) : 0.0f;
}

void engine::add_loopable(std::shared_ptr<::loopable> ptr) {
  _loopables.emplace_back(std::move(ptr));
}

#ifdef EMSCRIPTEN
template <class T>
inline void run(void* userdata) {
  reinterpret_cast<T*>(userdata)->_loop();
}
#endif

void engine::run() {
#ifdef EMSCRIPTEN
  emscripten_set_main_loop_arg(::run<engine>, this, 0, true);
#else
  while (_running) [[likely]] {
    _loop();
  }
#endif
}

void engine::_loop() {
  const auto now = SDL_GetPerformanceCounter();
  static auto prior = now;
  static const auto frequency = static_cast<double>(SDL_GetPerformanceFrequency());
  const auto delta = std::min(static_cast<float>(static_cast<double>(now - prior) / frequency), MAX_DELTA);
  prior = now;

  if (_tick_interval > 0.0f) {
    _tick_accumulator += delta;
    while (_tick_accumulator >= _tick_interval) {
      _tick_accumulator -= _tick_interval;
      _tick_current = (_tick_current % _ticks) + 1;
      _scenemanager->on_tick(_tick_current);
    }
  }

  for (const auto& observer : _observers) {
    observer->on_beginupdate();
  }

  _eventmanager->update(delta);
  _scenemanager->update(delta);
  _overlay->update(delta);

  for (const auto& loopable : _loopables) {
    loopable->loop(delta);
  }

  for (const auto& observer : _observers) {
    observer->on_endupdate();
  }

  for (const auto& observer : _observers) {
    observer->on_begindraw();
  }

  _renderer->begin();
  _scenemanager->draw();
  _overlay->draw();
  _canvas->draw();
  _renderer->end();

  for (const auto& observer : _observers) {
    observer->on_enddraw();
  }

#ifdef HAS_STEAM
  SteamAPI_RunCallbacks();
#endif
}

void engine::on_quit() noexcept {
  _running = false;
}
