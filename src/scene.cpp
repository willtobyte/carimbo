#include "scene.hpp"

#include "entityproxy.hpp"
#include "geometry.hpp"
#include "particlesystem.hpp"
#include "scenemanager.hpp"
#include "soundfx.hpp"

scene::scene(std::string_view scene, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager)
    : _renderer(std::move(scenemanager->renderer())),
      _scenemanager(std::move(scenemanager)),
      _particlesystem(scenemanager->resourcemanager()),
      _timermanager(std::make_shared<::timermanager>()) {
  _hits.reserve(64);

  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);

  const auto& soundmanager = _scenemanager->resourcemanager()->soundmanager();
  const auto& pixmappool = _scenemanager->resourcemanager()->pixmappool();
  const auto& fontfactory = _scenemanager->resourcemanager()->fontfactory();

  const auto es = json.value("effects", nlohmann::json::array());
  _effects.reserve(es.size());
  for (const auto& e : es) {
    const auto& name = e.get_ref<const std::string&>();
    const auto f = std::format("blobs/{}/{}.ogg", scene, name);
    _effects.emplace(name, soundmanager->get(f));
  }

  _background = pixmappool->get(std::format("blobs/{}/background.png", scene));

  const auto os = json.value("objects", nlohmann::json::array());

  int zindex = 0;
  for (const auto& o : os) {
    const auto name = o.at("name").get<std::string>();
    const auto kind = o.at("kind").get<std::string>();
    const auto action = make_action(o.value("action", std::string{}));

    const auto x = o.value("x", .0f);
    const auto y = o.value("y", .0f);

    const auto filename = std::format("objects/{}/{}.json", scene, kind);
    const auto buffer = io::read(filename);
    const auto j = nlohmann::json::parse(buffer);

    auto entity = _registry.create();

    metadata m;
    m.kind = make_action(kind);
    _registry.emplace<metadata>(entity, std::move(m));

    tint tn;
    _registry.emplace<tint>(entity, std::move(tn));

    sprite s;
    s.pixmap = std::move(pixmappool->get(std::format("blobs/{}/{}.png", scene, kind)));
    _registry.emplace<sprite>(entity, std::move(s));

    playback pb;
    pb.action = action;
    pb.dirty = true;
    pb.tick = SDL_GetTicks();
    pb.timeline = nullptr;
    _registry.emplace<playback>(entity, std::move(pb));

    transform tf;
    tf.position = {x, y};
    tf.angle = .0;
    tf.scale = j.value("scale", 1.0f);
    _registry.emplace<transform>(entity, std::move(tf));

    atlas at;
    for (auto& [key, value] : j.at("timelines").items()) {
      timeline tl;
      from_json(value, tl);
      at.timelines.emplace(make_action(key), std::move(tl));
    }

    _registry.emplace<atlas>(entity, std::move(at));

    orientation ori;
    _registry.emplace<orientation>(entity, std::move(ori));

    physics ph;
    _registry.emplace<physics>(entity, std::move(ph));

    renderable rn;
    rn.z = zindex++;
    _registry.emplace<renderable>(entity, std::move(rn));

    const auto e = std::make_shared<entityproxy>(entity, _registry);
    _proxies.emplace(std::move(name), e);

    callbacks cb;
    cb.self = e;
    _registry.emplace<callbacks>(entity, std::move(cb));
  }

  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });

  const auto ps = json.value("particles", nlohmann::json::array());
  _particles.reserve(ps.size());
  const auto factory = _particlesystem.factory();
  for (const auto& i : ps) {
    const auto particle = i.at("name").get<std::string_view>();
    const auto kind = i.at("kind").get<std::string_view>();
    const auto x = i.at("x").get<float>();
    const auto y = i.at("y").get<float>();
    const auto active = i.value("active", true);
    const auto batch = factory->create(kind, x, y, active);
    _particles.emplace(particle, batch);
    _particlesystem.add(batch);
  }

  const auto fs = json.value("fonts", nlohmann::json::array());
  for (const auto& i : fs) {
    const auto fontname = i.get<std::string_view>();
    fontfactory->get(fontname);
  }
}

scene::~scene() noexcept {
  auto view = _registry.view<physics>();
  for (auto entity : view) {
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

  _animationsystem.update(_registry, now);
  _physicssystem.update(_registry, _world, delta);
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

  _rendersystem.draw(_registry);

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
  if (kind == scenekind::object) {
    const auto it = _proxies.find(name);
    assert(it != _proxies.end() && "entity proxy not found in scene");
    return it->second;
  }

  if (kind == scenekind::effect) {
    const auto it = _effects.find(name);
    assert(it != _effects.end() && "effect not found in scene");
    return it->second;
  }

  if (kind == scenekind::particle) {
    const auto it = _particles.find(name);
    assert(it != _particles.end() && "particles not found in scene");
    return it->second->props;
  }

  std::terminate();
}

void scene::set_onenter(std::function<void()>&& fn) {
  _onenter = std::move(fn);
}

void scene::set_onloop(sol::protected_function&& fn) {
  _onloop = std::move(fn);
}

void scene::set_oncamera(sol::protected_function&& fn) {
  _oncamera = std::move(fn);
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

  for (auto entity : _hits) {
    if (auto* callback = _registry.try_get<callbacks>(entity); callback && callback->on_touch) {
      if (const auto fn = callback->on_touch; fn) {
        fn(callback->self, x, y);
      }
    }
  }
}

void scene::on_motion(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  for (const auto entity : _hovering) {
    if (_hits.contains(entity)) continue;
    if (auto* callback = _registry.try_get<callbacks>(entity); callback && callback->on_unhover) {
      if (const auto fn = callback->on_unhover; fn) {
        fn(callback->self);
      }
    }
  }

  for (const auto entity : _hits) {
    if (_hovering.contains(entity)) continue;
    if (auto* callback = _registry.try_get<callbacks>(entity); callback && callback->on_hover) {
      if (const auto fn = callback->on_hover; fn) {
        fn(callback->self);
      }
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
