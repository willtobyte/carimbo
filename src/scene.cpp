#include "scene.hpp"

using namespace framework;

scene::scene(
  std::shared_ptr<objectmanager> objectmanager,
  std::shared_ptr<graphics::pixmap> background,
  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects,
  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects,
  std::optional<std::shared_ptr<tilemap>> tilemap,
  geometry::size size
)
  : _objectmanager(objectmanager),
    _background(std::move(background)),
    _objects(std::move(objects)),
    _effects(std::move(effects)),
    _tilemap(std::move(tilemap)),
    _size(std::move(size)) {
}

scene::~scene() {
  const auto objects = std::exchange(_objects, {});

  for (const auto& [_, o] : objects) {
    _objectmanager->destroy(o);
  }

  const auto effects = std::exchange(_effects, {});
  for (const auto& [_, e] : effects) {
    e->stop();
  }

  std::println("[scene destroyed] >>>>>>>>>>>");
  _background.reset();
}

void scene::update(float_t delta) noexcept {
  if (const auto fn = _onloop; fn) {
    fn(delta);
  }

  if (const auto& tilemap = _tilemap.value_or(nullptr)) {
    tilemap->update(delta);
  }
}

void scene::draw() const noexcept {
  const auto background_width  = static_cast<float_t>(_background->width());
  const auto background_height = static_cast<float_t>(_background->height());

  const auto scene_width  = _size.width();
  const auto scene_height = _size.height();

  const geometry::size background_size{background_width, background_height};
  static geometry::point source_origin{0.0f, 0.0f};

  for (auto y = 0.0f; y < scene_height; y += background_height) {
    for (auto x = 0.0f; x < scene_width; x += background_width) {
      const geometry::point destination_position{x, y};
      _background->draw({destination_position, background_size}, {source_origin, background_size});
    }
  }

  if (const auto& tilemap = _tilemap.value_or(nullptr)) {
    tilemap->draw();
  }
}

std::variant<std::shared_ptr<object>, std::shared_ptr<audio::soundfx>> scene::get(const std::string& name, scenetype type) const {
  if (type == scenetype::object) {
    for (const auto& [key, obj] : _objects) {
      if (key == name) return obj;
    }
  }

  if (type == scenetype::effect) {
    for (const auto& [key, fx] : _effects) {
      if (key == name) return fx;
    }
  }

  throw std::out_of_range("scene::get(): '" + name + "' not found");
}

void scene::on_enter() const {
  for (const auto& [_, o] : _objects) {
    _objectmanager->manage(o);
  }

  if (const auto fn = _onenter) {
    fn();
  }
}

void scene::on_leave() const {
  if (const auto fn = _onleave) {
    fn();
  }

  for (const auto& [_, o] : _objects) {
    _objectmanager->unmanage(o);
  }

  for (const auto& [_, e] : _effects) {
    e->stop();
  }
}

void scene::on_touch(float_t x, float_t y) const {
  if (const auto fn = _ontouch) fn(x, y);
}

void scene::on_key_press(int32_t code) const {
  if (const auto fn = _onkeypress) fn(code);
}

void scene::on_key_release(int32_t code) const {
  if (const auto fn = _onkeyrelease) fn(code);
}

void scene::on_text(const std::string& text) const {
  if (const auto fn = _ontext) fn(text);
}

// void scene::on_mouse_press(float_t x, float_t y) {
//   if (const auto fn = _onmousepress) fn(x, y);
// }

// void scene::on_mouse_relese(float_t x, float_t y) {
//   if (const auto fn = _onmouserelease) fn(x, y);
// }

void scene::on_motion(float_t x, float_t y) const {
  if (const auto fn = _onmotion) fn(x, y);
}

void scene::set_onenter(std::function<void()>&& fn) {
  _onenter = std::move(fn);
}

void scene::set_onloop(std::function<void(float_t)>&& fn) {
  _onloop = std::move(fn);
}

void scene::set_onleave(std::function<void()>&& fn) {
  _onleave = std::move(fn);
}

void scene::set_ontouch(std::function<void(float_t, float_t)>&& fn) {
  _ontouch = std::move(fn);
}

void scene::set_onkeypress(std::function<void(int32_t)>&& fn) {
  _onkeypress = std::move(fn);
}

void scene::set_onkeyrelease(std::function<void(int32_t)>&& fn) {
  _onkeyrelease = std::move(fn);
}

void scene::set_ontext(std::function<void(const std::string& )>&& fn) {
  _ontext = std::move(fn);
}

// void scene::set_onmousepress(std::function<void(float_t, float_t)> fn) {
// _onmousepress = std::move(fn);
// }

// void scene::set_onmouserelease(std::function<void(float_t, float_t)> fn) {
// _onmouserelease = std::move(fn);
// }

void scene::set_onmotion(std::function<void(float_t, float_t)>&& fn) {
  _onmotion = std::move(fn);
}
