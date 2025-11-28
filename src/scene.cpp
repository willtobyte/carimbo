#include "scene.hpp"

#include "entityproxy.hpp"
#include "geometry.hpp"
#include "particlesystem.hpp"
#include "scenemanager.hpp"
#include "soundfx.hpp"
#include "tilemap.hpp"

scene::scene(std::string_view scene, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager)
    : _renderer(scenemanager->renderer()), _scenemanager(std::move(scenemanager)) {
  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);

  const auto& soundmanager = _scenemanager->resourcemanager()->soundmanager();
  const auto& pixmappool = _scenemanager->resourcemanager()->pixmappool();
  const auto& fontfactory = _scenemanager->resourcemanager()->fontfactory();
  const auto& particlesystem = _scenemanager->particlesystem();

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
    const auto name = o["name"].get<std::string>();
    const auto kind = o["kind"].get<std::string>();
    const auto q = o.value("action", std::string{});
    const auto action = q.empty() ? std::nullopt : std::optional<std::string>(q);

    const auto x = o.value("x", .0f);
    const auto y = o.value("y", .0f);

    const auto filename = std::format("objects/{}/{}.json", scene, kind);
    const auto buffer = io::read(filename);
    const auto j = nlohmann::json::parse(buffer);

    auto entity = _registry.create();

    metadata m;
    m.kind = kind;
    _registry.emplace<metadata>(entity, std::move(m));

    tint tn;
    _registry.emplace<tint>(entity, std::move(tn));

    sprite s;
    s.pixmap = std::move(pixmappool->get(std::format("blobs/{}/{}.png", scene, kind)));
    _registry.emplace<sprite>(entity, std::move(s));

    state st;
    st.action = action;
    st.dirty = true;
    st.tick = SDL_GetTicks();
    _registry.emplace<state>(entity, std::move(st));

    transform t;
    t.position = {x, y};
    t.angle = .0;
    t.scale = 1.0f;
    _registry.emplace<transform>(entity, std::move(t));

    animator an;
    for (auto& [key, value] : j["timelines"].items()) {
      timeline tl;
      from_json(value, tl);
      an.timelines.emplace(key, std::move(tl));
    }

    _registry.emplace<animator>(entity, std::move(an));

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
  const auto factory = particlesystem->factory();
  for (const auto& i : ps) {
    const auto particle = i["name"].get<std::string_view>();
    const auto kind = i["kind"].get<std::string_view>();
    const auto x = i["x"].get<float>();
    const auto y = i["y"].get<float>();
    const auto active = i.value("active", true);
    _particles.emplace(particle, factory->create(kind, x, y, active));
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

  if (auto fn = _onloop; fn) [[likely]] {
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

  auto view = _registry.view<renderable, transform, tint, sprite, animator, state>();

  for (auto entity : view) {
    const auto& [rn, tr, tn, sp, an, st] = view.get<renderable, transform, tint, sprite, animator, state>(entity);
    if (!st.action.has_value()) [[unlikely]] continue;

    const auto& timeline = an[st.action.value()];
    if (timeline.frames.empty()) [[unlikely]] continue;

    const auto& frame = timeline.frames[st.current_frame];

    const auto sw = frame.quad.w * tr.scale;
    const auto sh = frame.quad.h * tr.scale;

    const auto cx = frame.offset.x + tr.position.x + frame.quad.w * 0.5f;
    const auto cy = frame.offset.y + tr.position.y + frame.quad.h * 0.5f;

    const auto fx = cx - sw * 0.5f;
    const auto fy = cy - sh * 0.5f;

    sp.pixmap->draw(
      frame.quad.x, frame.quad.y,
      frame.quad.w, frame.quad.h,
      fx, fy,
      sw, sh,
      tr.angle,
      tn.a
    );
  }

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

void scene::set_onloop(sol::protected_function fn) {
  _onloop = interop::wrap_fn<void(float)>(std::move(fn));
}

void scene::set_oncamera(sol::protected_function fn) {
  _oncamera = interop::wrap_fn<quad(float)>(std::move(fn));
}

void scene::set_onleave(std::function<void()>&& fn) {
  _onleave = std::move(fn);
}

void scene::set_ontouch(sol::protected_function fn) {
  _ontouch = interop::wrap_fn<void(float, float)>(std::move(fn));
}

void scene::set_onkeypress(sol::protected_function fn) {
  _onkeypress = interop::wrap_fn<void(int32_t)>(std::move(fn));
}

void scene::set_onkeyrelease(sol::protected_function fn) {
  _onkeyrelease = interop::wrap_fn<void(int32_t)>(std::move(fn));
}

void scene::set_ontext(sol::protected_function fn) {
  _ontext = interop::wrap_fn<void(std::string_view)>(std::move(fn));
}

void scene::set_onmotion(sol::protected_function fn) {
  _onmotion = interop::wrap_fn<void(float, float)>(std::move(fn));
}

void scene::on_enter() const {
  const auto& particlesystem = _scenemanager->particlesystem();

  for (const auto& [key, batch] : _particles) {
    particlesystem->add(batch);
  }

  if (auto fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() const {
    if (auto fn = _onleave; fn) {
      fn();
    }

    for (const auto& [_, e] : _effects) {
      e->stop();
    }

    const auto& particlesystem = _scenemanager->particlesystem();
    particlesystem->clear();
}

void scene::on_touch(float x, float y) const {
  static std::vector<uint64_t> hits;
  hits.reserve(32);
  hits.clear();
  query(x, y, std::back_inserter(hits));
  if (hits.empty()) {
    if (auto fn = _ontouch; fn) {
      fn(x, y);
    }

    return;
  }

  for (auto id : hits) {
    if (const auto entity = find(id)) [[likely]] {
      if (_registry.all_of<callbacks>(*entity)) {
        if (auto callback = _registry.get<callbacks>(*entity); callback.on_touch) {
          callback.on_touch(callback.self, x, y);
        }
      }
    }
  }
}

void scene::on_motion(float x, float y) const {
  static std::unordered_set<uint64_t> hits;
  hits.reserve(32);
  hits.clear();
  query(x, y, std::inserter(hits, hits.end()));

  for (const auto id : _hovering) {
    if (hits.contains(id)) continue;
    if (const auto entity = find(id)) [[likely]] {
      if (_registry.all_of<callbacks>(*entity)) {
        if (auto callback = _registry.get<callbacks>(*entity); callback.on_unhover) {
          callback.on_unhover(callback.self);
        }
      }
    }
  }

  for (const auto id : hits) {
    if (_hovering.contains(id)) continue;
    if (const auto entity = find(id)) [[likely]] {
      if (_registry.all_of<callbacks>(*entity)) {
        if (auto callback = _registry.get<callbacks>(*entity); callback.on_hover) {
          callback.on_hover(callback.self);
        }
      }
    }
  }

  _hovering.swap(hits);

  if (auto fn = _onmotion; fn) {
    fn(x, y);
  }
}

void scene::on_key_press(int32_t code) const {
  if (auto fn = _onkeypress; fn) {
    fn(code);
  }
}

void scene::on_key_release(int32_t code) const {
  if (auto fn = _onkeyrelease; fn) {
    fn(code);
  }
}

void scene::on_text(std::string_view text) const {
  if (auto fn = _ontext; fn) {
    fn(text);
  }
}

std::optional<entt::entity> scene::find(uint64_t id) const {
  const auto entity = static_cast<entt::entity>(id);

  if (_registry.valid(entity)) {
    return entity;
  }

  return std::nullopt;
}
