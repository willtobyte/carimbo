#include "enginefactory.hpp"

#include "audiodevice.hpp"
#include "common.hpp"
#include "engine.hpp"
#include "entitymanager.hpp"
#include "eventmanager.hpp"
#include "resourcemanager.hpp"
#include "scenemanager.hpp"
#include "window.hpp"
#include "world.hpp"

using namespace framework;

enginefactory &enginefactory::with_title(const std::string &title) noexcept {
  _title = title;
  return *this;
}

enginefactory &enginefactory::with_width(int32_t width) noexcept {
  _width = width;
  return *this;
}

enginefactory &enginefactory::with_height(int32_t height) noexcept {
  _height = height;
  return *this;
}

enginefactory &enginefactory::with_scale(float_t scale) noexcept {
  _scale = scale;
  return *this;
}

enginefactory &enginefactory::with_gravity(float_t gravity) noexcept {
  _gravity = gravity;
  return *this;
}

enginefactory &enginefactory::with_fullscreen(bool fullscreen) noexcept {
  _fullscreen = fullscreen;
  return *this;
}

std::shared_ptr<engine> enginefactory::create() const noexcept {
  const auto audiodevice = std::make_shared<audio::audiodevice>();
  const auto engine = std::make_shared<framework::engine>();
  const auto eventmanager = std::make_shared<input::eventmanager>();
  const auto window = std::make_shared<graphics::window>(_title, _width, _height, _fullscreen);
  const auto renderer = window->create_renderer(_scale);
  const auto resourcemanager = std::make_shared<framework::resourcemanager>(renderer, audiodevice);
  const auto overlay = std::make_shared<graphics::overlay>(renderer);
  const auto statemanager = std::make_shared<framework::statemanager>();
  const auto entitymanager = std::make_shared<framework::entitymanager>(resourcemanager);
  const auto fontfactory = std::make_shared<graphics::fontfactory>(renderer);
  const auto scenemanager = std::make_shared<framework::scenemanager>(resourcemanager->pixmappool());

  engine->set_audiodevice(std::move(audiodevice));
  engine->set_entitymanager(std::move(entitymanager));
  engine->set_eventmanager(std::move(eventmanager));
  engine->set_overlay(std::move(overlay));
  engine->set_renderer(std::move(renderer));
  engine->set_resourcemanager(std::move(resourcemanager));
  engine->set_scenemanager(std::move(scenemanager));
  engine->set_statemanager(std::move(statemanager));
  engine->set_window(std::move(window));
  engine->set_fontfactory(fontfactory);

  engine->eventmanager()->add_receiver(engine->entitymanager());
  engine->eventmanager()->add_receiver(engine);
  engine->eventmanager()->add_receiver(engine->statemanager());

  return engine;
}
