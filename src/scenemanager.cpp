#include "scenemanager.hpp"

#include "event.hpp"
#include "io.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "resourcemanager.hpp"
#include "scene.hpp"
#include "timermanager.hpp"

using namespace framework;

scenemanager::scenemanager(
  std::shared_ptr<framework::resourcemanager> resourcemanager,
  std::shared_ptr<objectmanager> objectmanager,
  std::shared_ptr<graphics::particlesystem> particlesystem,
  std::shared_ptr<framework::timermanager> timermanager
)
  : _resourcemanager(std::move(resourcemanager)),
    _objectmanager(std::move(objectmanager)),
    _particlesystem(std::move(particlesystem)),
    _timermanager(std::move(timermanager)) {
}

std::shared_ptr<scene> scenemanager::load(std::string_view name) {
  const auto [it, inserted] = _scene_mapping.try_emplace(std::string(name));
  if (!inserted) [[unlikely]] {
    return nullptr;
  }

  const auto filename = std::format("scenes/{}.json", name);
  const auto buffer = storage::io::read(filename);
  const auto j = nlohmann::json::parse(buffer);

  auto background = _resourcemanager->pixmappool()->get(std::format("blobs/{}/background.png", name));

  geometry::size size{j.at("width").get<float>(), j.at("height").get<float>()};

  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects;
  const auto es = j.value("effects", nlohmann::json::array());
  effects.reserve(es.size());

  for (const auto& e : es) {
    const auto basename = e.get<std::string_view>();
    const auto f = std::format("blobs/{}/{}.ogg", name, basename);
    effects.emplace_back(basename, _resourcemanager->soundmanager()->get(f));
  }

  const auto ps = j.value("particles", nlohmann::json::array());
  std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> particles;
  particles.reserve(ps.size());

  const auto factory = _particlesystem->factory();

  for (const auto& i : ps) {
    const auto name = i["name"].get<std::string_view>();
    const auto kind = i["kind"].get<std::string_view>();
    const auto x = i["x"].get<float>();
    const auto y = i["y"].get<float>();
    const auto emitting = i.value("emitting", true);

    particles.emplace(name, factory->create(kind, x, y, emitting));
  }

  const auto fs = j.value("fonts", nlohmann::json::array());
  for (const auto& i : fs) {
    const auto fontname = i.get<std::string_view>();
    _resourcemanager->fontfactory()->get(fontname);
  }

  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects;
  const auto os = j.value("objects", nlohmann::json::array());
  objects.reserve(os.size());

  for (const auto& o : os) {
    if (!o.is_object()) [[unlikely]] {
      continue;
    }

    std::string key = o["name"].get<std::string>();
    std::string kind = o["kind"].get<std::string>();

    const float x = o.value("x", .0f);
    const float y = o.value("y", .0f);

    std::string action = o.value("action", std::string{});

    auto object = _objectmanager->create(kind, name, false);
    object->set_placement(x, y);
    if (!action.empty()) {
      object->set_action(action);
    }

    objects.emplace_back(std::move(key), std::move(object));
  }

  // std::optional<std::shared_ptr<tilemap>> map;
  // if (const auto it = j.find("tilemap"); it != j.end()) {
  //   map.emplace(std::make_shared<tilemap>(size, _resourcemanager, it->get<std::string>()));
  // }

  return it->second = std::make_shared<scene>(
    name,
    _objectmanager,
    _particlesystem,
    std::move(background),
    std::move(objects),
    std::move(effects),
    std::move(particles),
    // std::move(map),
    std::move(size)
  );
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
    _timermanager->clear();
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

void scenemanager::on_touch(float x, float y) const {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_touch(x, y);
}

void scenemanager::on_key_press(const input::event::keyboard::key& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_key_press(static_cast<int32_t>(event));
}

void scenemanager::on_key_release(const input::event::keyboard::key& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_key_release(static_cast<int32_t>(event));
}

void scenemanager::on_text(std::string_view text) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_text(text);
}

void scenemanager::on_mouse_press(const input::event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
}

void scenemanager::on_mouse_release(const input::event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
}

void scenemanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_motion(event.x, event.y);
}
