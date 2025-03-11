#include "scenemanager.hpp"

using namespace framework;

scenemanager::scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool, std::shared_ptr<entitymanager> entitymanager) noexcept
    : _pixmappool(pixmappool), _entitymanager(entitymanager) {}

void scenemanager::set(const std::string &name) noexcept {
  _background.reset();
  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = nlohmann::json::parse(buffer);
  _background = _pixmappool->get(j["background"].get_ref<const std::string &>());
  _size = {j.at("width").get<int32_t>(), j.at("height").get<int32_t>()};

  const auto &entities = j["entities"];
  std::transform(
      entities.begin(),
      entities.end(),
      std::inserter(_entities, _entities.end()),
      [](const auto &item) -> std::pair<std::string, std::shared_ptr<entity>> {
        return {item.key(), entitymanager::create_entity_from_json(item.value())};
      }
  );
}

void scenemanager::grab(const std::string &key) noexcept {
  UNUSED(key);
}

void scenemanager::update(float_t delta) noexcept {
  UNUSED(delta);
}

void scenemanager::draw() const noexcept {
  if (!_background) {
    return;
  }

  static geometry::point point{0, 0};
  _background->draw({point, _size}, {point, _size});
}
