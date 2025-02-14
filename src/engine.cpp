#include "engine.hpp"

#include "audiodevice.hpp"
#include "entitymanager.hpp"
#include "eventmanager.hpp"
#include "framerate.hpp"
#include "loopable.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"
#include "statemanager.hpp"
#include "window.hpp"
#include "world.hpp"

using namespace framework;

engine::engine() noexcept
    : _running(true) {
  add_loopable(std::make_shared<framerate>());
}

std::shared_ptr<audio::audiodevice> engine::audiodevice() const noexcept {
  return _audiodevice;
}

std::shared_ptr<framework::entitymanager> engine::entitymanager() const noexcept {
  return _entitymanager;
}

std::shared_ptr<input::eventmanager> engine::eventmanager() const noexcept {
  return _eventmanager;
}

std::shared_ptr<graphics::fontfactory> engine::fontfactory() const noexcept {
  return _fontfactory;
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

int32_t engine::height() const noexcept {
  return _window->height();
}

int32_t engine::width() const noexcept {
  return _window->width();
}

void engine::set_audiodevice(std::shared_ptr<audio::audiodevice> audiodevice) noexcept {
  _audiodevice = std::move(audiodevice);
}

void engine::set_entitymanager(std::shared_ptr<framework::entitymanager> entitymanager) noexcept {
  _entitymanager = std::move(entitymanager);
}

void engine::set_eventmanager(std::shared_ptr<input::eventmanager> eventmanager) noexcept {
  _eventmanager = std::move(eventmanager);
}

void engine::set_fontfactory(std::shared_ptr<graphics::fontfactory> fontfactory) noexcept {
  _fontfactory = std::move(fontfactory);
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
}

void engine::add_loopable(std::shared_ptr<loopable> loopable) noexcept {
  _loopables.emplace_back(std::move(loopable));
}

void engine::flush() const noexcept {
  _resourcemanager->flush();
}

// bool engine::is_keydown(const input::keyevent &event) const noexcept {
//   return _statemanager->is_keydown(event);
// }

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
  static auto prior = SDL_GetTicks();
  const auto now = SDL_GetTicks();
  const auto delta = std::min(static_cast<float_t>(now - prior) / 1000.0f, 1.0f / 60.0f);

  prior = now;

  for (const auto &observer : _observers) {
    observer->on_beginupdate();
  }

  _resourcemanager->update(delta);
  _scenemanager->update(delta);
  _eventmanager->update(delta);
  _overlay->update(delta);
  _entitymanager->update(delta);

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
  _entitymanager->draw();
  _overlay->draw();
  _renderer->end();

  for (const auto &observer : _observers) {
    observer->on_enddraw();
  }
}

void engine::on_quit() noexcept {
  _running = false;
}
