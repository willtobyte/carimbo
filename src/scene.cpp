#include "scene.hpp"

#include "entityproxy.hpp"
#include "geometry.hpp"
#include "particlesystem.hpp"
#include "scenemanager.hpp"
#include "soundfx.hpp"

scene::scene(std::string_view scene, unmarshal::document& document, std::weak_ptr<::scenemanager> scenemanager)
    : _scenemanager(scenemanager),
      _particlesystem(scenemanager.lock()->resourcemanager()) {
  const auto locked = _scenemanager.lock();
  assert(locked && "scenemanager expired");

  _renderer = locked->renderer();
  _timermanager = std::make_shared<::timermanager>();

  _hits.reserve(64);

  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);

  const auto& soundmanager = locked->resourcemanager()->soundmanager();
  const auto& pixmappool = locked->resourcemanager()->pixmappool();
  const auto& fontfactory = locked->resourcemanager()->fontfactory();

  if (auto effects = unmarshal::find_array(document, "effects")) {
    for (auto element : *effects) {
      const auto name = unmarshal::string(element);
      const auto path = std::format("blobs/{}/{}.ogg", scene, name);
      _effects.emplace(name, soundmanager->get(path));
    }
  }

  _background = pixmappool->get(std::format("blobs/{}/background.png", scene));

  auto zindex = 0;
  if (auto objects = unmarshal::find_array(document, "objects")) {
    for (auto element : *objects) {
      auto object = unmarshal::get<unmarshal::object>(element);
      const auto name = unmarshal::get<std::string_view>(object, "name");
      const auto kind = unmarshal::get<std::string_view>(object, "kind");
      const auto action = make_action(unmarshal::value_or(object, "action", std::string_view{}));

      const auto x = unmarshal::value_or(object, "x", .0f);
      const auto y = unmarshal::value_or(object, "y", .0f);

      const auto filename = std::format("objects/{}/{}.json", scene, kind);
      const auto j = unmarshal::parse(io::read(filename)); auto& dobject = *j;

      const auto entity = _registry.create();

      atlas at{};
      for (auto field : dobject["timelines"].get_object()) {
        at.timelines.emplace(make_action(unmarshal::key(field)), unmarshal::make<timeline>(field.value()));
      }

      _registry.emplace<atlas>(entity, std::move(at));

      metadata md{
        .kind = make_action(kind)
      };
      _registry.emplace<metadata>(entity, std::move(md));

      _registry.emplace<tint>(entity);

      sprite sp{
        .pixmap = pixmappool->get(std::format("blobs/{}/{}.png", scene, kind))
      };
      _registry.emplace<sprite>(entity, std::move(sp));

      playback pb{
        .dirty = true,
        .redraw = false,
        .current_frame = 0,
        .tick = SDL_GetTicks(),
        .action = action,
        .timeline = nullptr
      };
      _registry.emplace<playback>(entity, std::move(pb));

      transform tr{
        .position = vec2{x, y},
        .angle = .0,
        .scale = unmarshal::value_or(dobject, "scale", 1.0f)
      };
      _registry.emplace<transform>(entity, std::move(tr));

      _registry.emplace<orientation>(entity);

      _registry.emplace<physics>(entity);

      renderable rd{
        .z = zindex++
      };
      _registry.emplace<renderable>(entity, std::move(rd));

      const auto proxy = std::make_shared<entityproxy>(entity, _registry);
      _proxies.emplace(std::move(name), proxy);

      callbacks c {
        .self = proxy
      };
      _registry.emplace<callbacks>(entity, std::move(c));
    }

    _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
      return lhs.z < rhs.z;
    });
  }

  const auto factory = _particlesystem.factory();
  if (auto particles = unmarshal::find_array(document, "particles")) {
    for (auto element : *particles) {
      auto object = unmarshal::get<unmarshal::object>(element);
      const auto name = unmarshal::get<std::string_view>(object, "name");
      const auto kind = unmarshal::get<std::string_view>(object, "kind");
      const auto px = unmarshal::get<float>(object, "x");
      const auto py = unmarshal::get<float>(object, "y");
      const auto active = unmarshal::value_or(object, "active", true);
      const auto batch = factory->create(kind, px, py, active);
      _particles.emplace(name, batch);
      _particlesystem.add(batch);
    }
  }

  if (auto fonts = unmarshal::find_array(document, "fonts")) {
    for (auto element : *fonts) {
      auto fontname = unmarshal::string(element);
      fontfactory->get(fontname);
    }
  }
}

scene::~scene() noexcept {
  const auto view = _registry.view<physics>();
  for (const auto entity : view) {
    auto& ph = view.get<physics>(entity);
    if (b2Shape_IsValid(ph.shape)) {
      b2DestroyShape(ph.shape, false);
    }

    if (ph.is_valid()) {
      b2DestroyBody(ph.body);
    }
  }

  if (b2World_IsValid(_world)) {
    b2DestroyWorld(_world);
  }
}

void scene::update(float delta) noexcept {
  const auto now = SDL_GetTicks();

  _animationsystem.update(now);
  _physicssystem.update(_world, delta);
  _particlesystem.update(delta);

  if (const auto fn = _onloop; fn) {
    fn(delta);
  }
}

#ifdef DEBUG
[[nodiscard]] static bool _draw_callback(const b2ShapeId shape, void* const ctx) {
  auto* const renderer = static_cast<SDL_Renderer*>(ctx);
  const auto aabb = b2Shape_GetAABB(shape);

  const auto r = SDL_FRect{
    aabb.lowerBound.x,
    aabb.lowerBound.y,
    aabb.upperBound.x - aabb.lowerBound.x,
    aabb.upperBound.y - aabb.lowerBound.y
  };

  SDL_RenderRect(renderer, &r);
  return true;
};
#endif

void scene::draw() const noexcept {
  const auto w = static_cast<float>(_background->width());
  const auto h = static_cast<float>(_background->height());

  _background->draw(.0f, .0f, w, h, .0f, .0f, w, h);

  _rendersystem.draw();

  _particlesystem.draw();

#ifdef DEBUG
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 255, 255);

  int width, height;
  SDL_GetRenderOutputSize(*_renderer, &width, &height);

  const auto x0 = .0f;
  const auto y0 = .0f;
  const auto x1 = static_cast<float>(width);
  const auto y1 = static_cast<float>(height);

  b2AABB aabb{{x0 - epsilon, y0 - epsilon}, {x1 + epsilon, y1 + epsilon}};
  const auto filter = b2DefaultQueryFilter();
  b2World_OverlapAABB(_world, aabb, filter, _draw_callback, static_cast<SDL_Renderer*>(*_renderer));
#endif
}

std::variant<
  std::shared_ptr<entityproxy>,
  std::shared_ptr<soundfx>,
  std::shared_ptr<particleprops>
> scene::get(std::string_view name, scenekind kind) const {
  switch (kind) {
    case scenekind::object: {
      const auto it = _proxies.find(name);
      assert(it != _proxies.end() && "entity proxy not found in scene");
      return it->second;
    }

    case scenekind::effect: {
      const auto it = _effects.find(name);
      assert(it != _effects.end() && "effect not found in scene");
      return it->second;
    }

    case scenekind::particle: {
      const auto it = _particles.find(name);
      assert(it != _particles.end() && "particles not found in scene");
      return it->second->props;
    }

    default:
      std::terminate();
  }
}

void scene::set_onenter(std::function<void()>&& fn) {
  _onenter = std::move(fn);
}

void scene::set_onloop(sol::protected_function&& fn) {
  _onloop = std::move(fn);
}

void scene::set_onleave(std::function<void()>&& fn) {
  _onleave = std::move(fn);
}

void scene::set_ontouch(sol::protected_function&& fn) {
  _ontouch = std::move(fn);
}

void scene::set_onkeypress(sol::protected_function&& fn) {
  _onkeypress = std::move(fn);
}

void scene::set_onkeyrelease(sol::protected_function&& fn) {
  _onkeyrelease = std::move(fn);
}

void scene::set_ontext(sol::protected_function&& fn) {
  _ontext = std::move(fn);
}

void scene::set_onmotion(sol::protected_function&& fn) {
  _onmotion = std::move(fn);
}

void scene::set_oncamera([[maybe_unused]] sol::protected_function&& fn) {
}

void scene::on_enter() {
  if (auto fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() {
  _timermanager->clear();

  for (const auto& [_, e] : _effects) {
    e->stop();
  }

  if (auto fn = _onleave; fn) {
    fn();
  }
}

void scene::on_touch(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  if (_hits.empty()) {
    if (const auto fn = _ontouch; fn) {
      fn(x, y);
    }
    return;
  }

  for (const auto entity : _hits) {
    if (const auto* c = _registry.try_get<callbacks>(entity); c && c->on_touch) {
      c->on_touch(c->self, x, y);
    }
  }
}

void scene::on_motion(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  for (const auto entity : _hovering) {
    if (_hits.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity); c && c->on_unhover) {
      c->on_unhover(c->self);
    }
  }

  for (const auto entity : _hits) {
    if (_hovering.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity); c && c->on_hover) {
      c->on_hover(c->self);
    }
  }

  _hovering.swap(_hits);

  if (const auto fn = _onmotion; fn) {
    fn(x, y);
  }
}

void scene::on_key_press(int32_t code) {
  if (const auto fn = _onkeypress; fn) {
    fn(code);
  }
}

void scene::on_key_release(int32_t code) {
  if (const auto fn = _onkeyrelease; fn) {
    fn(code);
  }
}

void scene::on_text(std::string_view text) {
  if (const auto fn = _ontext; fn) {
    fn(text);
  }
}

std::shared_ptr<timermanager> scene::timermanager() const noexcept {
  return _timermanager;
}
