#include "scene.hpp"

using namespace framework;

scene::scene(std::shared_ptr<graphics::pixmap> background, std::unordered_map<std::string, std::shared_ptr<object>> objects, geometry::size size) noexcept
    : _background(std::move(background)),
      _objects(std::move(objects)),
      _size(std::move(size)) {}

void scene::update(float_t delta) noexcept {
  UNUSED(delta);
}

void scene::draw() const noexcept {
  static geometry::point point{0, 0};
  _background->draw({point, _size}, {point, _size});

  for (const auto &o : _objects) {
    o.second->draw();
  }
}
