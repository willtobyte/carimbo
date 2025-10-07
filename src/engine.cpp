#include "engine.hpp"

#include "audiodevice.hpp"
#include "eventmanager.hpp"
#include "loopable.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "statemanager.hpp"
#include "window.hpp"

using namespace framework;

std::shared_ptr<audio::audiodevice> engine::audiodevice() const noexcept {
  return _audiodevice;
}

std::shared_ptr<framework::objectmanager> engine::objectmanager() const noexcept {
  return _objectmanager;
}

std::shared_ptr<input::eventmanager> engine::eventmanager() const noexcept {
  return _eventmanager;
}

std::shared_ptr<graphics::fontfactory> engine::fontfactory() const noexcept {
  return _resourcemanager->fontfactory();
}

std::shared_ptr<graphics::overlay> engine::overlay() const noexcept {
  return _overlay;
}

std::shared_ptr<framework::resourcemanager> engine::resourcemanager() const noexcept {
  return _resourcemanager;
}

std::shared_ptr<framework::scenemanager> engine::scenemanager() const noexcept {
  return _scenemanager;
}

std::shared_ptr<framework::statemanager> engine::statemanager() const noexcept {
  return _statemanager;
}

std::shared_ptr<audio::soundmanager> engine::soundmanager() const noexcept {
  return _resourcemanager->soundmanager();
}

std::shared_ptr<graphics::window> engine::window() const noexcept {
  return _window;
}

std::shared_ptr<graphics::renderer> engine::renderer() const noexcept {
  return _renderer;
}

std::shared_ptr<graphics::particlesystem> engine::particlesystem() const noexcept {
  return _particlesystem;
}

std::shared_ptr<graphics::canvas> engine::canvas() const noexcept {
  return _canvas;
}

std::shared_ptr<storage::cassette> engine::cassette() const noexcept {
  return _cassette;
}

std::shared_ptr<framework::timermanager> engine::timermanager() const noexcept {
  return _timermanager;
}

int32_t engine::height() const noexcept {
  return _window->height();
}

int32_t engine::width() const noexcept {
  return _window->width();
}

void engine::set_audiodevice(std::shared_ptr<audio::audiodevice> audiodevice) noexcept {
  _audiodevice = std::move(audiodevice);
}

void engine::set_objectmanager(std::shared_ptr<framework::objectmanager> objectmanager) noexcept {
  _objectmanager = std::move(objectmanager);
}

void engine::set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager) noexcept {
  _eventmanager = std::move(eventmanager);
}

void engine::set_overlay(std::shared_ptr<graphics::overlay> overlay) noexcept {
  _overlay = std::move(overlay);
}

void engine::set_resourcemanager(std::shared_ptr<framework::resourcemanager> resourcemanager) noexcept {
  _resourcemanager = std::move(resourcemanager);
}

void engine::set_scenemanager(std::shared_ptr<framework::scenemanager> scenemanager) noexcept {
  _scenemanager = std::move(scenemanager);
}

void engine::set_particlesystem(std::shared_ptr<graphics::particlesystem> particlesystem) noexcept {
  _particlesystem = std::move(particlesystem);
}

void engine::set_statemanager(std::shared_ptr<framework::statemanager> statemanager) noexcept {
  _statemanager = std::move(statemanager);

  _observers.emplace_back(_statemanager);
}

void engine::set_window(std::shared_ptr<graphics::window> window) noexcept {
  _window = std::move(window);
}

void engine::set_renderer(std::shared_ptr<graphics::renderer> renderer) noexcept {
  _renderer = std::move(renderer);

  _canvas = std::make_shared<graphics::canvas>(_renderer);
}

void engine::add_loopable(std::shared_ptr<loopable> loopable) noexcept {
  _loopables.emplace_back(std::move(loopable));
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
  const auto ticks = SDL_GetTicks();
  static auto prior = ticks;
  const auto delta = static_cast<float_t>(ticks - prior) * 0.001f;
  prior = ticks;

  for (const auto& observer : _observers) {
    observer->on_beginupdate();
  }

  _eventmanager->update(delta);
  _resourcemanager->update(delta);
  _overlay->update(delta);
  _scenemanager->update(delta);
  _objectmanager->update(delta);
  _particlesystem->update(delta);

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
  _objectmanager->draw();
  _particlesystem->draw();
  _overlay->draw();
  _canvas->draw();
  _renderer->end();

  for (const auto& observer : _observers) {
    observer->on_enddraw();
  }

  #ifdef HAVE_STEAM
  SteamAPI_RunCallbacks();
  #endif
}

void engine::on_quit() {
  _running = false;
}

#ifdef DEBUG
void engine::on_debug() {
  _resourcemanager->debug();
}
#endif
