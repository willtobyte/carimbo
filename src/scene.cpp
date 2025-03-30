#include "scene.hpp"

using namespace framework;

scene::scene(std::shared_ptr<objectmanager> objectmanager, std::shared_ptr<graphics::pixmap> background, std::unordered_map<std::string, std::shared_ptr<object>> objects, geometry::size size) noexcept
    : _objectmanager(objectmanager),
      _background(std::move(background)),
      _objects(std::move(objects)),
      _size(std::move(size)) {}

scene::~scene() noexcept {
  for (const auto &o : _objects) {
    _objectmanager->destroy(o.second);
  }

  _objects.clear();
  _background.reset();
}

void scene::update(float_t delta) noexcept {
  if (const auto fn = _onloop; fn) {
    fn(delta);
  }
}

void scene::draw() const noexcept {
  static geometry::point point{0, 0};
  _background->draw({point, _size}, {point, _size});

  for (const auto &o : _objects) {
    o.second->draw();
  }
}

std::shared_ptr<object> scene::get(const std::string &name) const noexcept {
  return _objects.at(name);
}

void scene::on_enter() noexcept {
  if (const auto fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() noexcept {
  if (const auto fn = _onleave; fn) {
    fn();
  }
}

void scene::set_onenter(std::function<void()> fn) noexcept {
  _onenter = std::move(fn);
}

void scene::set_onloop(std::function<void(float_t)> fn) noexcept {
  _onloop = std::move(fn);
}

void scene::set_onleave(std::function<void()> fn) noexcept {
  _onleave = std::move(fn);
}
