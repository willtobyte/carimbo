#include "engine.hpp"

#include "audiodevice.hpp"
#include "canvas.hpp"
#include "cassette.hpp"
#include "constant.hpp"
#include "eventmanager.hpp"
#include "eventreceiver.hpp"
#include "lifecycleobserver.hpp"
#include "loopable.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "postalservice.hpp"
#include "scenemanager.hpp"
#include "statemanager.hpp"
#include "timermanager.hpp"
#include "window.hpp"
#include "world.hpp"

std::shared_ptr<audiodevice> engine::audiodevice() const noexcept {
  return _audiodevice;
}

std::shared_ptr<objectmanager> engine::objectmanager() const noexcept {
  return _objectmanager;
}

std::shared_ptr<eventmanager> engine::eventmanager() const noexcept {
  return _eventmanager;
}

std::shared_ptr<fontfactory> engine::fontfactory() const noexcept {
  return _resourcemanager->fontfactory();
}

std::shared_ptr<overlay> engine::overlay() const noexcept {
  return _overlay;
}

std::shared_ptr<resourcemanager> engine::resourcemanager() const noexcept {
  return _resourcemanager;
}

std::shared_ptr<postalservice> engine::postalservice() const noexcept {
  return _postalservice;
}

std::shared_ptr<scenemanager> engine::scenemanager() const noexcept {
  return _scenemanager;
}

std::shared_ptr<statemanager> engine::statemanager() const noexcept {
  return _statemanager;
}

std::shared_ptr<soundmanager> engine::soundmanager() const noexcept {
  return _resourcemanager->soundmanager();
}

std::shared_ptr<window> engine::window() const noexcept {
  return _window;
}

std::shared_ptr<renderer> engine::renderer() const noexcept {
  return _renderer;
}

std::shared_ptr<particlesystem> engine::particlesystem() const noexcept {
  return _particlesystem;
}

std::shared_ptr<canvas> engine::canvas() const noexcept {
  return _canvas;
}

std::shared_ptr<cassette> engine::cassette() const noexcept {
  return _cassette;
}

std::shared_ptr<timermanager> engine::timermanager() const noexcept {
  return _timermanager;
}

std::shared_ptr<world> engine::world() const noexcept {
  return _world;
}

void engine::set_audiodevice(std::shared_ptr<::audiodevice> ptr) {
  _audiodevice = std::move(ptr);
}

void engine::set_objectmanager(std::shared_ptr<::objectmanager> ptr) {
  _objectmanager = std::move(ptr);
}

void engine::set_eventmanager(std::shared_ptr<::eventmanager> ptr) {
  _eventmanager = std::move(ptr);
}

void engine::set_overlay(std::shared_ptr<::overlay> ptr) {
  _overlay = std::move(ptr);
}

void engine::set_resourcemanager(std::shared_ptr<::resourcemanager> ptr) {
  _resourcemanager = std::move(ptr);
}

void engine::set_scenemanager(std::shared_ptr<::scenemanager> ptr) {
  _scenemanager = std::move(ptr);
}

void engine::set_particlesystem(std::shared_ptr<::particlesystem> ptr) {
  _particlesystem = std::move(ptr);
}

void engine::set_postalservice(std::shared_ptr<::postalservice> ptr) {
  _postalservice = std::move(ptr);
}

void engine::set_statemanager(std::shared_ptr<::statemanager> ptr) {
  _statemanager = std::move(ptr);

  _observers.emplace_back(_statemanager);
}

void engine::set_window(std::shared_ptr<::window> ptr) {
  _window = std::move(ptr);
}

void engine::set_world(std::shared_ptr<::world> ptr) {
  _world = std::move(ptr);
}

void engine::set_renderer(std::shared_ptr<::renderer> ptr) {
  _renderer = std::move(ptr);

  _canvas = std::make_shared<::canvas>(_renderer);
}

void engine::set_timermanager(std::shared_ptr<::timermanager> ptr) {
  _timermanager = std::move(ptr);
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
  const auto ticks = SDL_GetTicks();
  static auto prior = ticks;
  const auto delta = static_cast<float>(ticks - prior) * 0.001f;
  prior = ticks;

  for (const auto& observer : _observers) {
    observer->on_beginupdate();
  }

  _eventmanager->update(delta);
  _resourcemanager->update(delta);
  _overlay->update(delta);
  _scenemanager->update(delta);
  _world->update(delta);
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
  _world->draw();
  _particlesystem->draw();
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

#ifndef NDEBUG
void engine::on_debug() {
  _resourcemanager->debug();
}
#endif
