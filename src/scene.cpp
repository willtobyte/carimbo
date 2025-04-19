#include "scene.hpp"

using namespace framework;

scene::scene(
    std::shared_ptr<objectmanager> objectmanager,
    std::shared_ptr<graphics::pixmap> background,
    std::unordered_map<std::string, std::shared_ptr<object>> objects,
    std::vector<std::shared_ptr<audio::soundfx>> effects,
    geometry::size size
) noexcept
    : _objectmanager(objectmanager),
      _background(std::move(background)),
      _objects(std::move(objects)),
      _effects(std::move(effects)),
      _size(std::move(size)) {
}

scene::~scene() noexcept {
  const auto objects = std::exchange(_objects, {});

  for (const auto &[_, o] : objects) {
    _objectmanager->destroy(o);
  }

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
}

std::shared_ptr<object> scene::get(const std::string &name) const noexcept {
  return _objects.at(name);
}

void scene::on_enter() noexcept {
  for (auto &[_, o] : _objects) {
    _objectmanager->manage(o);
  }

  if (const auto fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() noexcept {
  if (const auto fn = _onleave; fn) {
    fn();
  }

  for (auto &[_, o] : _objects) {
    _objectmanager->unmanage(o);
  }
}

void scene::on_touch(float_t x, float_t y) const noexcept {
  if (const auto fn = _ontouch; fn) {
    fn(x, y);
  }
}

void scene::on_motion(float_t x, float_t y) const noexcept {
  if (const auto fn = _onmotion; fn) {
    fn(x, y);
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

void scene::set_ontouch(std::function<void(float_t, float_t)> fn) noexcept {
  _ontouch = std::move(fn);
}

void scene::set_onmotion(std::function<void(float_t, float_t)> fn) noexcept {
  _onmotion = std::move(fn);
}
