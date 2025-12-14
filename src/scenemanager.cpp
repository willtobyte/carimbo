#include "scenemanager.hpp"

#include "event.hpp"
#include "io.hpp"
#include "resourcemanager.hpp"
#include "scene.hpp"

scenemanager::scenemanager(std::shared_ptr<::resourcemanager> resourcemanager, std::shared_ptr<::renderer> renderer)
    : _resourcemanager(std::move(resourcemanager)),
      _renderer(std::move(renderer)) {
}

std::shared_ptr<scene> scenemanager::load(std::string_view name) {
  const auto [it, inserted] = _scene_mapping.try_emplace(name);
  if (inserted) {
    const auto filename = std::format("scenes/{}.json", name);
    auto json = unmarshal::parse(io::read(filename)); auto& document = *json;

    return it->second = std::make_shared<scene>(name, document, shared_from_this());
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

  if (_scene) [[likely]] {
    std::println("[scenemanager] left {}", _scene->name());
    _scene->on_leave();
  }

  _scene = it->second.get();
  _current = name;

  std::println("[scenemanager] entered {}", name);

  _scene->on_enter();
}

boost::container::small_vector<std::string, 8> scenemanager::query(std::string_view name) const {
  boost::container::small_vector<std::string, 8> result;
  const bool all = name.size() == 1 && name.front() == '*';
  if (all) {
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

boost::container::small_vector<std::string, 8> scenemanager::destroy(std::string_view name) {
  const auto scenes = query(name);

  for (const auto& scene : scenes) {
    if (_scene_mapping.erase(scene) > 0) {
      std::println("[scenemanager] destroyed {}", scene);
      _resourcemanager->flush();
    }
  }

  return scenes;
}

void scenemanager::update(float delta) {
  _scene->update(delta);
}

void scenemanager::draw() const {
  _scene->draw();
}

void scenemanager::on_key_press(const event::keyboard::key& event) {
  _scene->on_key_press(static_cast<int32_t>(event));
}

void scenemanager::on_key_release(const event::keyboard::key& event) {
  _scene->on_key_release(static_cast<int32_t>(event));
}

void scenemanager::on_text(std::string_view text) {
  _scene->on_text(text);
}

void scenemanager::on_mouse_press(const event::mouse::button& event) {
}

void scenemanager::on_mouse_release(const event::mouse::button& event) {
  _scene->on_touch(event.x, event.y);
}

void scenemanager::on_mouse_motion(const event::mouse::motion& event) {
  _scene->on_motion(event.x, event.y);
}

std::shared_ptr<resourcemanager> scenemanager::resourcemanager() const noexcept {
  return _resourcemanager;
}

std::shared_ptr<::renderer> scenemanager::renderer() const noexcept {
  return _renderer;
}
