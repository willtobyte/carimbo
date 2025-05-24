#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<framework::resourcemanager> resourcemanager, std::shared_ptr<objectmanager> objectmanager)
  : _resourcemanager(std::move(resourcemanager)),
    _objectmanager(std::move(objectmanager)) {
}

std::shared_ptr<scene> scenemanager::load(const std::string &name) {
  const auto &filename = fmt::format("scenes/{}.json", name);
  const auto &buffer = storage::io::read(filename);
  const auto &j = nlohmann::json::parse(buffer);

  const auto pixmappool = _resourcemanager->pixmappool();
  const auto background = pixmappool->get(fmt::format("blobs/{}/background.png", name));
  geometry::size size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};

  const auto &es = j.value("effects", nlohmann::json::array());
  const auto eview = es
    | std::views::transform([&](const auto& e) {
      const auto& basename = e.template get<std::string>();
      auto path = std::format("blobs/{}/{}.ogg", name, basename);
      return std::pair{
        basename,
        _resourcemanager->soundmanager()->get(path)
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

      auto e = _objectmanager->create(name, kind, false);
      e->set_placement(x, y);
      if (action) {
        e->set_action(*action);
      }

      return std::pair<std::string, std::shared_ptr<object>>{key, e};
    });

  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects;
  objects.reserve(os.size());
  std::ranges::copy(oview, std::inserter(objects, objects.end()));

  const auto s = std::make_shared<scene>(
    _objectmanager,
    background,
    std::move(objects),
    std::move(effects),
    size
  );

  _scene_mapping[name] = s;
  return s;
}

void scenemanager::set(const std::string &name) {
  if (_scene) {
    _scene->on_leave();
  }

  _scene = _scene_mapping.at(name);

  _scene->on_enter();
}

std::shared_ptr<scene> scenemanager::get(const std::string &name) const {
  return _scene_mapping.at(name);
}

void scenemanager::destroy(const std::string &name) {
  _scene_mapping.erase(name);
}

void scenemanager::on_touch(float_t x, float_t y) const {
  _scene->on_touch(x, y);
}

void scenemanager::update(float_t delta) {
  _scene->update(delta);
}

void scenemanager::draw() const {
  _scene->draw();
}

void scenemanager::on_key_press(const input::event::keyboard::key &event) {
  UNUSED(event);
}

void scenemanager::on_key_release(const input::event::keyboard::key &event) {
  UNUSED(event);
}

void scenemanager::on_text(const std::string &text) {
  _scene->on_text(text);
}

void scenemanager::on_mouse_press(const input::event::mouse::button &event) {
  UNUSED(event);
}

void scenemanager::on_mouse_release(const input::event::mouse::button &event) {
  UNUSED(event);
}

void scenemanager::on_mouse_motion(const input::event::mouse::motion &event) {
  _scene->on_motion(event.x, event.y);
}
