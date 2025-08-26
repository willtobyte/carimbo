#include "enginefactory.hpp"

#include "audiodevice.hpp"
#include "common.hpp"
#include "engine.hpp"
#include "eventmanager.hpp"
#include "objectmanager.hpp"
#include "resourcemanager.hpp"
#include "scenemanager.hpp"
#include "window.hpp"

using namespace framework;

enginefactory& enginefactory::with_title(const std::string& title) noexcept {
  _title = title;
  return *this;
}

enginefactory& enginefactory::with_width(int32_t width) noexcept {
  _width = width;
  return *this;
}

enginefactory& enginefactory::with_height(int32_t height) noexcept {
  _height = height;
  return *this;
}

enginefactory& enginefactory::with_scale(float_t scale) noexcept {
  _scale = scale;
  return *this;
}

enginefactory& enginefactory::with_gravity(float_t gravity) noexcept {
  _gravity = gravity;
  return *this;
}

enginefactory& enginefactory::with_fullscreen(bool fullscreen) noexcept {
  _fullscreen = fullscreen;
  return *this;
}

std::shared_ptr<engine> enginefactory::create() const {
  const auto audiodevice = std::make_shared<audio::audiodevice>();
  const auto engine = std::make_shared<framework::engine>();
  const auto window = std::make_shared<graphics::window>(_title, _width, _height, _fullscreen);
  const auto renderer = window->create_renderer(_scale);
  const auto eventmanager = std::make_shared<input::eventmanager>(renderer);
  const auto resourcemanager = std::make_shared<framework::resourcemanager>(renderer, audiodevice, engine);
  const auto overlay = std::make_shared<graphics::overlay>(resourcemanager, eventmanager);
  const auto statemanager = std::make_shared<framework::statemanager>();
  const auto objectmanager = std::make_shared<framework::objectmanager>(resourcemanager);
  const auto scenemanager = std::make_shared<framework::scenemanager>(resourcemanager, objectmanager);

  engine->set_audiodevice(audiodevice);
  engine->set_objectmanager(objectmanager);
  engine->set_eventmanager(eventmanager);
  engine->set_overlay(overlay);
  engine->set_renderer(renderer);
  engine->set_resourcemanager(resourcemanager);
  engine->set_scenemanager(scenemanager);
  engine->set_statemanager(statemanager);
  engine->set_window(window);

  engine->eventmanager()->add_receiver(engine->objectmanager());
  engine->eventmanager()->add_receiver(engine);
  engine->eventmanager()->add_receiver(engine->statemanager());
  engine->eventmanager()->add_receiver(overlay);
  engine->eventmanager()->add_receiver(std::static_pointer_cast<input::eventreceiver>(scenemanager));

  objectmanager->set_scenemanager(scenemanager);

  return engine;
}
