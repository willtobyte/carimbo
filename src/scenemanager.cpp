#include "scenemanager.hpp"
#include "entity.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool, std::shared_ptr<entitymanager> entitymanager) noexcept
    : _pixmappool(std::move(pixmappool)), _entitymanager(std::move(entitymanager)) {}

void scenemanager::set(const std::string &name) noexcept {
  if (!_current_scene.empty()) {
    if (auto it = _onleave_mapping.find(_current_scene); it != _onleave_mapping.end()) {
      it->second();
    }
  }

  _current_scene = name;

  _background.reset();

  const auto old = std::exchange(_entities, {});
  for (const auto &pair : old) {
    _entitymanager->destroy(pair.second);
  }

  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = nlohmann::json::parse(buffer);

  _background = _pixmappool->get(j["background"].get_ref<const std::string &>());
  _size = {j.at("width").get<int32_t>(), j.at("height").get<int32_t>()};

  const auto &es = j.value("entities", nlohmann::json::array());
  const auto &i = es.items();
  _entities.reserve(es.size());
  std::transform(
      i.begin(),
      i.end(),
      std::inserter(_entities, _entities.end()),
      [&](const auto &item) -> std::pair<std::string, std::shared_ptr<entity>> {
        const auto &key = item.key();
        const auto &data = item.value();
        const auto &kind = data["kind"].template get_ref<const std::string &>();
        const auto &action = data["action"].template get_ref<const std::string &>();
        const auto x = data.value("x", 0);
        const auto y = data.value("y", 0);

        auto e = _entitymanager->spawn(kind);
        e->set_placement(x, y);
        e->set_action(action);
        return {key, e};
      }
  );

  if (auto it = _onenter_mapping.find(name); it != _onenter_mapping.end()) {
    it->second();
  }
}

void scenemanager::update(float_t delta) noexcept {
  UNUSED(delta);
}

void scenemanager::draw() const noexcept {
  if (!_background) [[unlikely]] {
    return;
  }

  static geometry::point point{0, 0};
  _background->draw({point, _size}, {point, _size});
}

std::shared_ptr<entity> scenemanager::grab(const std::string &key) const noexcept {
  if (const auto it = _entities.find(key); it != _entities.end()) {
    return it->second;
  }

  return nullptr;
}

void scenemanager::set_onenter(const std::string &name, std::function<void()> fn) {
  _onenter_mapping[name] = std::move(fn);
}

void scenemanager::set_onleave(const std::string &name, std::function<void()> fn) {
  _onleave_mapping[name] = std::move(fn);
}
