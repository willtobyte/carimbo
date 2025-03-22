#include "scenemanager.hpp"
#include "entity.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool, std::shared_ptr<entitymanager> entitymanager) noexcept
    : _pixmappool(pixmappool), _entitymanager(entitymanager) {}

void scenemanager::set(const std::string &name) noexcept {
  _background.reset();

  const auto old = std::exchange(_entities, {});
  for (const auto &pair : old) {
    _entitymanager->destroy(pair.second);
  }

  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = nlohmann::json::parse(buffer);

  _background = _pixmappool->get(j["background"].get_ref<const std::string &>());
  _size = {j.at("width").get<int32_t>(), j.at("height").get<int32_t>()};

  const auto &es = j.value("entities", nlohmann::json::object());
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
        const auto x = data["x"].template get<int32_t>();
        const auto y = data["y"].template get<int32_t>();

        auto e = _entitymanager->spawn(kind);
        e->set_placement(x, y);
        e->set_action(action);
        return {key, e};
      }
  );
}

std::shared_ptr<entity> scenemanager::grab(const std::string &key) const noexcept {
  if (const auto it = _entities.find(key); it != _entities.end()) {
    return it->second;
  }

  return nullptr;
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
