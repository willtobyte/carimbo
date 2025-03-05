#include "scenemanager.hpp"

using namespace framework;

using json = nlohmann::json;

scenemanager::scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool) noexcept
    : _pixmappool(std::move(pixmappool)) {}

void scenemanager::set(const std::string &name) noexcept {
  _background.reset();
  const auto buffer = storage::io::read("scenes/" + name + ".json");
  const auto j = json::parse(buffer);
  _background = _pixmappool->get(j["background"].get_ref<const std::string &>());
  _size = {j.at("width").get<int32_t>(), j.at("height").get<int32_t>()};
}

void scenemanager::grab(const std::string &key) noexcept {
  UNUSED(key);
}

void scenemanager::update(float_t delta) noexcept {
  UNUSED(delta);
}

void scenemanager::draw() const noexcept {
  if (not _background) {
    return;
  }

  static geometry::point point{0, 0};
  _background->draw({point, _size}, {point, _size});
}
