#include "scenemanager.hpp"

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

std::shared_ptr<scene> scenemanager::load(const std::string& name) {
  const auto [it, inserted] = _scene_mapping.try_emplace(name, nullptr);
  if (!inserted) {
    return nullptr;
  }

  const auto& filename = std::format("scenes/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  auto background = _resourcemanager->pixmappool()->get(std::format("blobs/{}/background.png", name));

  geometry::size size{j.at("width").get<float>(), j.at("height").get<float>()};

  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects;
  const auto es = j.value("effects", nlohmann::json::array());
  effects.reserve(es.size());

  for (const auto& e : es) {
    const auto& basename = e.get_ref<const std::string&>();
    const auto f = std::format("blobs/{}/{}.ogg", name, basename);
    effects.emplace_back(basename, _resourcemanager->soundmanager()->get(f));
  }

  const auto ps = j.value("particles", nlohmann::json::array());
  std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> particles;
  particles.reserve(ps.size());

  const auto factory = _particlesystem->factory();

  for (const auto& i : ps) {
    const auto& name = i["name"].get_ref<const std::string&>();
    const auto& kind = i["kind"].get_ref<const std::string&>();
    const auto x = i["x"].get<float>();
    const auto y = i["y"].get<float>();

    particles[name] = factory->create(kind, x, y);
  }

  const auto fs = j.value("fonts", nlohmann::json::array());
  for (const auto& i : fs) {
    _resourcemanager->fontfactory()->get(i.get_ref<const std::string&>());
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

    const float x = o.value("x", 0.0f);
    const float y = o.value("y", 0.0f);

    std::string action = o.value("action", std::string{});

    auto object = _objectmanager->create(kind, name, false);
    object->set_placement(x, y);
    if (!action.empty()) {
      object->set_action(action);
    }

    objects.emplace_back(std::move(key), std::move(object));
  }

  std::optional<std::shared_ptr<tilemap>> map;
  if (const auto it = j.find("tilemap"); it != j.end()) {
    map.emplace(std::make_shared<tilemap>(size, _resourcemanager, *it));
  }

  return it->second = std::make_shared<scene>(
    name,
    _objectmanager,
    _particlesystem,
    std::move(background),
    std::move(objects),
    std::move(effects),
    std::move(particles),
    std::move(map),
    std::move(size)
  );
}

void scenemanager::set(const std::string& name) {
  if (const auto active = _scene.lock()) {
    std::println("[scenemanager] left {}", active->name());
    _timermanager->clear();
    active->on_leave();
  }

  const auto& ptr = _scene_mapping.at(name);
  _scene = ptr;
  _current = name;

  std::println("[scenemanager] entered {}", name);
  ptr->on_enter();
}

std::vector<std::string> scenemanager::destroy(const std::string& name) noexcept {
  std::vector<std::string> result;
  result.reserve(8);
  if (name.size() == 1 && name.front() == '*') {
    for (auto it = _scene_mapping.begin(); it != _scene_mapping.end(); ) {
      if (it->first == _current) { ++it; continue; }
      std::println("[scenemanager] destroyed {}", it->first);
      result.emplace_back(it->first);
      it = _scene_mapping.erase(it);
    }

    _resourcemanager->flush();
    return result;
  }

  if (_scene_mapping.erase(name) == 0) {
    std::println("[scenemanager] {} not found, nothing to destroy", name);
    return result;
  }

  std::println("[scenemanager] destroyed {}", name);
  result.emplace_back(name);

  _resourcemanager->flush();
  return result;
}

void scenemanager::update(float delta) noexcept {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->update(delta);
}

void scenemanager::draw() const noexcept {
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

void scenemanager::on_text(const std::string& text) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_text(text);
}

void scenemanager::on_mouse_press(const input::event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  UNUSED(event);
  UNUSED(ptr);
  // ptr->on_mouse_press(event.x, event.y);
}

void scenemanager::on_mouse_release(const input::event::mouse::button& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  UNUSED(event);
  UNUSED(ptr);
  // ptr->on_mouse_release(event.x, event.y);
}

void scenemanager::on_mouse_motion(const input::event::mouse::motion& event) {
  const auto ptr = _scene.lock();
  if (!ptr) [[unlikely]] return;
  ptr->on_motion(event.x, event.y);
}
