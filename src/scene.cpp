#include "scene.hpp"

using namespace framework;

scene::scene(
  const std::string& name,
  std::shared_ptr<objectmanager> objectmanager,
  std::shared_ptr<graphics::particlesystem> particlesystem,
  std::shared_ptr<graphics::pixmap> background,
  std::vector<std::pair<std::string, std::shared_ptr<object>>> objects,
  std::vector<std::pair<std::string, std::shared_ptr<audio::soundfx>>> effects,
  std::unordered_map<std::string, std::shared_ptr<graphics::particlebatch>> particles,
  std::optional<std::shared_ptr<tilemap>> tilemap,
  geometry::size size
)
  : _name(name),
    _objectmanager(std::move(objectmanager)),
    _particlesystem(std::move(particlesystem)),
    _background(std::move(background)),
    _objects(std::move(objects)),
    _effects(std::move(effects)),
    _particles(std::move(particles)),
    _tilemap(std::move(tilemap)),
    _size(std::move(size)) {
}

scene::~scene() {
  auto objects = std::exchange(_objects, {});
  for (const auto& [_, o] : objects) {
    _objectmanager->remove(o);
  }

  auto effects = std::exchange(_effects, {});
  for (const auto& [_, e] : effects) {
    e->stop();
  }

  _particlesystem->clear();
}

void scene::update(float delta) {
  if (const auto& fn = _onloop; fn) [[likely]] {
    fn(delta);
  }

  if (const auto& tilemap = _tilemap.value_or(nullptr)) {
    tilemap->update(delta);
  }
}

void scene::draw() const {
  const auto r =
    geometry::rectangle(
      .0f, .0f,
      static_cast<float>(_background->width()),
      static_cast<float>(_background->height())
    );

  _background->draw(r, r);
}

std::variant<
  std::shared_ptr<object>,
  std::shared_ptr<audio::soundfx>,
  std::shared_ptr<graphics::particleprops>
> scene::get(const std::string& id, scenetype type) const {
  if (type == scenetype::object) {
    for (const auto& [key, object] : _objects) {
      if (key == id) return object;
    }
  }

  if (type == scenetype::effect) {
    for (const auto& [key, effect] : _effects) {
      if (key == id) return effect;
    }
  }

  if (type == scenetype::particle) {
    for (const auto& [key, batch] : _particles) {
      if (key == id) return batch->props;
    }
  }

  throw std::out_of_range(std::format("[scene] resource {} not found", id));
}

std::string scene::name() const {
  return _name;
}

void scene::on_enter() const {
  for (const auto& [_, o] : _objects) {
    _objectmanager->manage(o);
  }

  for (const auto& [key, batch] : _particles) {
    _particlesystem->add(batch);
  }

  if (const auto& fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() const {
  if (const auto& fn = _onleave; fn) {
    fn();
  }

  _particlesystem->clear();

  for (const auto& [_, o] : _objects) {
    _objectmanager->remove(o);
  }

  for (const auto& [_, e] : _effects) {
    e->stop();
  }
}

void scene::on_touch(float x, float y) const {
  if (const auto& fn = _ontouch; fn) {
    fn(x, y);
  }
}

void scene::on_key_press(int32_t code) const {
  if (const auto& fn = _onkeypress; fn) {
    fn(code);
  }
}

void scene::on_key_release(int32_t code) const {
  if (const auto& fn = _onkeyrelease; fn) {
    fn(code);
  }
}

void scene::on_text(const std::string& text) const {
  if (const auto& fn = _ontext; fn) {
    fn(text);
  }
}



void scene::on_motion(float x, float y) const {
  if (const auto& fn = _onmotion; fn) {
    fn(x, y);
  }
}

void scene::set_onenter(std::function<void()>&& fn) {
  _onenter = std::move(fn);
}

void scene::set_onloop(std::function<void(float)>&& fn) {
  _onloop = std::move(fn);
}

void scene::set_onleave(std::function<void()>&& fn) {
  _onleave = std::move(fn);
}

void scene::set_ontouch(std::function<void(float, float)>&& fn) {
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



void scene::set_onmotion(std::function<void(float, float)>&& fn) {
  _onmotion = std::move(fn);
}
