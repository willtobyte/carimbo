#include "engine.hpp"

#include "audiodevice.hpp"
#include "eventmanager.hpp"
#include "loopable.hpp"
#include "objectmanager.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "statemanager.hpp"
#include "window.hpp"

using namespace framework;

std::shared_ptr<audio::audiodevice> engine::audiodevice() const {
  return _audiodevice;
}

std::shared_ptr<framework::objectmanager> engine::objectmanager() const {
  return _objectmanager;
}

std::shared_ptr<input::eventmanager> engine::eventmanager() const {
  return _eventmanager;
}

std::shared_ptr<graphics::fontfactory> engine::fontfactory() const {
  return _resourcemanager->fontfactory();
}

std::shared_ptr<graphics::overlay> engine::overlay() const {
  return _overlay;
}

std::shared_ptr<framework::resourcemanager> engine::resourcemanager() const {
  return _resourcemanager;
}

std::shared_ptr<framework::scenemanager> engine::scenemanager() const {
  return _scenemanager;
}

std::shared_ptr<framework::statemanager> engine::statemanager() const {
  return _statemanager;
}

std::shared_ptr<audio::soundmanager> engine::soundmanager() const {
  return _resourcemanager->soundmanager();
}

std::shared_ptr<graphics::window> engine::window() const {
  return _window;
}

std::shared_ptr<graphics::renderer> engine::renderer() const {
  return _renderer;
}

std::shared_ptr<graphics::canvas> engine::canvas() const {
  return _canvas;
}

std::shared_ptr<storage::cassette> engine::cassette() const {
  return _cassette;
}

std::shared_ptr<framework::timermanager> engine::timermanager() const {
  return _timermanager;
}

int32_t engine::height() const {
  return _window->height();
}

int32_t engine::width() const {
  return _window->width();
}

void engine::set_audiodevice(std::shared_ptr<audio::audiodevice> audiodevice) {
  _audiodevice = std::move(audiodevice);
}

void engine::set_objectmanager(std::shared_ptr<framework::objectmanager> objectmanager) {
  _objectmanager = std::move(objectmanager);
}

void engine::set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager) {
  _eventmanager = std::move(eventmanager);
}

void engine::set_overlay(std::shared_ptr<graphics::overlay> overlay) {
  _overlay = std::move(overlay);
}

void engine::set_resourcemanager(std::shared_ptr<framework::resourcemanager> resourcemanager) {
  _resourcemanager = std::move(resourcemanager);
}

void engine::set_scenemanager(std::shared_ptr<framework::scenemanager> scenemanager) {
  _scenemanager = std::move(scenemanager);
}

void engine::set_statemanager(std::shared_ptr<framework::statemanager> statemanager) {
  _statemanager = std::move(statemanager);

  _observers.emplace_back(_statemanager);
}

void engine::set_window(std::shared_ptr<graphics::window> window) {
  _window = std::move(window);
}

void engine::set_renderer(std::shared_ptr<graphics::renderer> renderer) {
  _renderer = std::move(renderer);

  _canvas = std::make_shared<graphics::canvas>(_renderer);
}

void engine::add_loopable(std::shared_ptr<loopable> loopable) {
  _loopables.emplace_back(std::move(loopable));
}

void engine::flush() const {
  _resourcemanager->flush();
}

void engine::prefetch() {
  _resourcemanager->prefetch();
}

void engine::prefetch(const std::vector<std::string> &filenames) {
  _resourcemanager->prefetch(filenames);
}

#ifdef EMSCRIPTEN
template <class T>
inline void run(void *arg) {
  reinterpret_cast<T *>(arg)->_loop();
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
  const auto delta = std::min(static_cast<float_t>(ticks - prior) * 0.001f, 1.0f / 60.0f);
  prior = ticks;

  for (const auto &observer : _observers) {
    observer->on_beginupdate();
  }

  _eventmanager->update(delta);
  _overlay->update(delta);
  _scenemanager->update(delta);
  _objectmanager->update(delta);

  for (const auto &loopable : _loopables) {
    loopable->loop(delta);
  }

  for (const auto &observer : _observers) {
    observer->on_endupdate();
  }

  for (const auto &observer : _observers) {
    observer->on_begindraw();
  }

  _renderer->begin();
  _scenemanager->draw();
  _objectmanager->draw();
  _overlay->draw();
  _canvas->draw();
  _renderer->end();

  for (const auto &observer : _observers) {
    observer->on_enddraw();
  }

  #ifdef HAVE_STEAM
  SteamAPI_RunCallbacks();
  #endif
}

void engine::on_quit() {
  _running = false;
}
