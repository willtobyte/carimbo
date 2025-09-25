#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager)
    : _resourcemanager(std::move(resourcemanager)),
      _objectmanager(std::move(objectmanager)) {
}

std::shared_ptr<scene> scenemanager::load(const std::string& name) {
  const auto [it, inserted] = _scene_mapping.try_emplace(name, nullptr);
  if (!inserted) {
    return nullptr;
  }

  const auto& filename = std::format("scenes/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto background = _resourcemanager->pixmappool()->get(std::format("blobs/{}/background.png", name));

  geometry::size size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};

  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects;
  const auto& es = j.value("effects", nlohmann::json::array());
  effects.reserve(es.size());

  for (const auto& i : es) {
    const std::string basename = i.get<std::string>();
    const std::string f = std::format("blobs/{}/{}.ogg", name, basename);
    effects.emplace_back(std::move(basename), _resourcemanager->soundmanager()->get(f));
  }

  const auto& fs = j.value("fonts", nlohmann::json::array());
  for (const auto& i : fs) {
    _resourcemanager->fontfactory()->get(i);
  }

  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects;
  const auto os = j.value("objects", nlohmann::json::array());
  objects.reserve(os.size());

  for (const auto& i : os) {
    if (!i.is_object()) continue;

    std::string key = i["name"].get<std::string>();
    std::string kind = i["kind"].get<std::string>();

    const float x = i.value("x", 0.0f);
    const float y = i.value("y", 0.0f);

    std::string action = i.value("action", std::string{});

    auto o = _objectmanager->create(kind, name, false);
    o->set_placement(x, y);
    if (!action.empty()) {
      o->set_action(action);
    }

    objects.emplace_back(std::move(key), std::move(o));
  }

  std::optional<std::shared_ptr<tilemap>> map;
  if (const auto it = j.find("tilemap"); it != j.end()) {
    map.emplace(std::make_shared<tilemap>(size, _resourcemanager, *it));
  }

  it->second = std::make_shared<scene>(
    name,
    _objectmanager,
    std::move(background),
    std::move(objects),
    std::move(effects),
    std::move(map),
    std::move(size)
  );

  return it->second;
}

void scenemanager::set(const std::string& name) {
  if (const auto active = _scene.lock()) {
    std::println("[scenemanager] left scene {}", active->name());
    active->on_leave();
  }

  const auto& ptr = _scene_mapping.at(name);
  _scene = ptr;
  _current = name;

  std::println("[scenemanager] entered scene {}", name);
  ptr->on_enter();
}

std::vector<std::string> scenemanager::destroy(const std::string& name) noexcept {
  std::vector<std::string> result;

  if (name.size() == 1 && name.front() == '*') {
    for (auto it = _scene_mapping.begin(); it != _scene_mapping.end(); ) {
      if (it->first == _current) { ++it; continue; }
      std::println("[scenemanager] destroyed scene {}", it->first);
      result.push_back(it->first);
      it = _scene_mapping.erase(it);
    }

    _resourcemanager->flush();
    return result;
  }

  if (_scene_mapping.erase(name) == 0) {
    std::println("[scenemanager] {} not found, nothing to destroy", name);
    return result;
  }

  std::println("[scenemanager] destroyed scene {}", name);
  result.push_back(name);

  _resourcemanager->flush();
  return result;
}

void scenemanager::update(float_t delta) noexcept {
  const auto s = _scene.lock();

  s->update(delta);
}

void scenemanager::draw() const noexcept {
  const auto s = _scene.lock();

  s->draw();
}

void scenemanager::on_touch(float_t x, float_t y) const {
  const auto s = _scene.lock();

  s->on_touch(x, y);
}

void scenemanager::on_key_press(const input::event::keyboard::key& event) {
  const auto s = _scene.lock();

  s->on_key_press(static_cast<int32_t>(event));
}

void scenemanager::on_key_release(const input::event::keyboard::key& event) {
  const auto s = _scene.lock();

  s->on_key_release(static_cast<int32_t>(event));
}

void scenemanager::on_text(const std::string& text) {
  const auto s = _scene.lock();

  s->on_text(text);
}

void scenemanager::on_mouse_press(const input::event::mouse::button& event) {
  const auto s = _scene.lock();

  UNUSED(event);
  UNUSED(s);
  // s->on_mouse_press(event.x, event.y);
}

void scenemanager::on_mouse_release(const input::event::mouse::button& event) {
  const auto s = _scene.lock();

  UNUSED(event);
  UNUSED(s);
  // s->on_mouse_release(event.x, event.y);
}

void scenemanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto s = _scene.lock();

  s->on_motion(event.x, event.y);
}
