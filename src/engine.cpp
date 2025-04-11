#include "engine.hpp"

#include "audiodevice.hpp"
#include "eventmanager.hpp"
#include "framerate.hpp"
#include "loopable.hpp"
#include "objectmanager.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "statemanager.hpp"
#include "window.hpp"

using namespace framework;

engine::engine() noexcept {
  add_loopable(std::make_shared<framerate>());
}

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

void engine::flush() const noexcept {
  _resourcemanager->flush();
}

void engine::prefetch() noexcept {
  _resourcemanager->prefetch();
}

void engine::prefetch(const std::vector<std::string> &filenames) noexcept {
  _resourcemanager->prefetch(filenames);
}

#ifdef EMSCRIPTEN
template <class T>
inline void run(void *arg) noexcept {
  reinterpret_cast<T *>(arg)->_loop();
}
#endif

void engine::run() noexcept {
#ifdef EMSCRIPTEN
  emscripten_set_main_loop_arg(::run<engine>, this, 0, true);
#else
  while (_running) [[likely]] {
    _loop();
  }
#endif
}

void engine::_loop() noexcept {
  const auto ticks = SDL_GetTicks();
  static auto prior = ticks;
  const auto delta = std::min(static_cast<float>(ticks - prior) * 0.001f, 1.0f / 60.0f);
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
}

void engine::on_quit() noexcept {
  _running = false;
}
