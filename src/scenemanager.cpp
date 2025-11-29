#include "scenemanager.hpp"

#include "event.hpp"
#include "io.hpp"
#include "resourcemanager.hpp"
#include "scene.hpp"

scenemanager::scenemanager(std::shared_ptr<::resourcemanager> resourcemanager, std::shared_ptr<::renderer> renderer)
  : _resourcemanager(std::move(resourcemanager)), _renderer(std::move(renderer)) {
}

std::shared_ptr<scene> scenemanager::load(std::string_view name) {
  const auto [it, inserted] = _scene_mapping.try_emplace(std::string{name});
  if (inserted) [[unlikely]] {
    const auto filename = std::format("scenes/{}.json", name);
    const auto buffer = io::read(filename);
    const auto j = nlohmann::json::parse(buffer);

    const auto type = j.at("type").get<scenetype>();

    switch(type) {
      case scenetype::backdrop:
        return it->second = std::make_shared<scene>(name, j, shared_from_this());
      // case scenetype::tilemap:
      //   return it->second = std::make_shared<scenetilemap>(j);
    }
  }

  return nullptr;
}

std::string_view scenemanager::current() const {
  return _current;
}

void scenemanager::set(std::string_view name) {
  if (_current == name) [[unlikely]] {
    std::println("[scenemanager] already in {}", name);
    return;
  }

  const auto it = _scene_mapping.find(name);
  if (it == _scene_mapping.end()) [[unlikely]] return;

  if (const auto active = _scene.lock()) [[ likely ]] {
    std::println("[scenemanager] left {}", active->name());
    active->on_leave();
  }

  const auto& ptr = it->second;
  _scene = ptr;
  _current = name;

  std::println("[scenemanager] entered {}", name);

  ptr->on_enter();
}

std::shared_ptr<scene> scenemanager::get() const {
  return _scene.lock();
}

std::vector<std::string> scenemanager::query(std::string_view name) const {
  std::vector<std::string> result;
  result.reserve(8);
  if (name.size() == 1 && name.front() == '*') {
    for (const auto& [key, _] : _scene_mapping) {
      if (key == _current) continue;
      result.emplace_back(key);
    }
    return result;
  }

  if (_scene_mapping.find(name) != _scene_mapping.end()) {
    result.emplace_back(name);
  }

  return result;
}

std::vector<std::string> scenemanager::destroy(std::string_view name) {
  const auto scenes = query(name);

  for (const auto& scene : scenes) {
    if (_scene_mapping.erase(scene) > 0) {
      std::println("[scenemanager] destroyed {}", scene);
    }
  }

  if (!scenes.empty()) {
    _resourcemanager->flush();
  }

  return scenes;
}

void scenemanager::update(float delta) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->update(delta);
}

void scenemanager::draw() const {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->draw();
}

void scenemanager::on_key_press(const event::keyboard::key& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_key_press(static_cast<int32_t>(event));
}

void scenemanager::on_key_release(const event::keyboard::key& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_key_release(static_cast<int32_t>(event));
}

void scenemanager::on_text(std::string_view text) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_text(text);
}

void scenemanager::on_mouse_press(const event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
}

void scenemanager::on_mouse_release(const event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_touch(event.x, event.y);
}

void scenemanager::on_mouse_motion(const event::mouse::motion& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_motion(event.x, event.y);
}

std::shared_ptr<resourcemanager> scenemanager::resourcemanager() const noexcept {
  return _resourcemanager;
}

std::shared_ptr<::renderer> scenemanager::renderer() const noexcept {
  return _renderer;
}
