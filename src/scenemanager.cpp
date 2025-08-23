#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager)
    : _resourcemanager(std::move(resourcemanager)),
      _objectmanager(std::move(objectmanager)) {
}

std::shared_ptr<scene> scenemanager::load(const std::string& name) {
  if (_scene_mapping.contains(name)) {
    return nullptr;
  }

  const auto& filename = std::format("scenes/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  const auto pixmappool = _resourcemanager->pixmappool();
  const auto background = pixmappool->get(std::format("blobs/{}/background.png", name));
  geometry::size size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};

  const auto& es = j.value("effects", nlohmann::json::array());
  const auto eview = es
    | std::views::transform([&](const auto& e) {
      const auto& basename = e.template get<std::string>();
      const auto f = std::format("blobs/{}/{}.ogg", name, basename);
      return std::pair{
        basename,
        _resourcemanager->soundmanager()->get(f)
      };
    });

  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects;
  effects.reserve(es.size());
  std::ranges::copy(eview, std::inserter(effects, effects.end()));

  const auto& os = j.value("objects", nlohmann::json::array());
  const auto oview = os
    | std::views::transform([&](const auto& data) {
      const auto& key = data["name"].template get_ref<const std::string&>();
      const auto& kind = data["kind"].template get_ref<const std::string&>();
      std::optional<std::string> action =
        data.contains("action") && data["action"].is_string()
          ? std::optional<std::string>{data["action"].template get_ref<const std::string&>()}
          : std::nullopt;
      const auto x = data.value("x", .0f);
      const auto y = data.value("y", .0f);

      auto e = _objectmanager->create(kind, name, false);
      e->set_placement(x, y);
      if (action) {
        e->set_action(*action);
      }

      return std::pair<std::string, std::shared_ptr<object>>{key, e};
    });

  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects;
  objects.reserve(os.size());
  std::ranges::copy(oview, std::inserter(objects, objects.end()));

  std::optional<std::shared_ptr<tilemap>> map;
  if (const auto it = j.find("tilemap"); it != j.end()) {
    map.emplace(std::make_shared<tilemap>(size, _resourcemanager, *it));
  }

  const auto s = std::make_shared<scene>(
    name,
    _objectmanager,
    std::move(background),
    std::move(objects),
    std::move(effects),
    std::move(map),
    size
  );

  _scene_mapping[name] = s;
  return s;
}

void scenemanager::set(const std::string& name) {
  if (auto active = _scene.lock()) {
    std::println("[scenemanager] left scene {}", active->name());
    active->on_leave();
  }

  auto& ptr = _scene_mapping.at(name);
  _scene = ptr;
  _current = name;

  std::println("[scenemanager] entered scene {}", name);
  ptr->on_enter();
}

void scenemanager::destroy(const std::string& name) {
  if (name.size() == 1 && name.front() == '*') {
    for (auto it = _scene_mapping.begin(); it != _scene_mapping.end(); ) {
      if (it->first == _current) { ++it; continue; }
      std::println("[scenemanager] destroyed scene {}", it->first);

      it = _scene_mapping.erase(it);
    }

    _resourcemanager->flush();
    return;
  }

  if (_scene_mapping.erase(name) == 0) {
    return;
  }

  std::println("[scenemanager] destroyed scene {}", name);

  _resourcemanager->flush();
}

void scenemanager::update(float_t delta) noexcept {
  auto s = _scene.lock();
  s->update(delta);
}

void scenemanager::draw() const noexcept {
  auto s = _scene.lock();
  s->draw();
}

void scenemanager::on_touch(float_t x, float_t y) const {
  auto s = _scene.lock();
  s->on_touch(x, y);
}

void scenemanager::on_key_press(const input::event::keyboard::key& event) {
  auto s = _scene.lock();
  s->on_key_press(static_cast<int32_t>(event));
}

void scenemanager::on_key_release(const input::event::keyboard::key& event) {
  auto s = _scene.lock();
  s->on_key_release(static_cast<int32_t>(event));
}

void scenemanager::on_text(const std::string& text) {
  auto s = _scene.lock();
  s->on_text(text);
}

void scenemanager::on_mouse_press(const input::event::mouse::button& event) {
  auto s = _scene.lock();
  UNUSED(event);
  // s->on_mouse_press(event.x, event.y);
}

void scenemanager::on_mouse_release(const input::event::mouse::button& event) {
  auto s = _scene.lock();
  UNUSED(event);
  // s->on_mouse_release(event.x, event.y);
}

void scenemanager::on_mouse_motion(const input::event::mouse::motion& event) {
  auto s = _scene.lock();
  s->on_motion(event.x, event.y);
}
