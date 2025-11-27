#include "scene.hpp"

#include "geometry.hpp"
#include "object.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "pixmap.hpp"
#include "scenemanager.hpp"
#include "soundfx.hpp"
#include "tilemap.hpp"

scene::scene(std::string_view scene, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager)
    : _renderer(scenemanager->renderer()) {
  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);

  const auto pixmappool = scenemanager->resourcemanager()->pixmappool();

  _background = pixmappool->get(std::format("blobs/{}/background.png", scene));

  const auto os = json.value("objects", nlohmann::json::array());

  for (const auto& o : os) {
    const auto name = o["name"].get<std::string_view>();
    const auto kind = o["kind"].get<std::string_view>();
    const auto action = o["action"].get<std::string_view>();

    const auto x = o.value("x", .0f);
    const auto y = o.value("y", .0f);

    const auto filename = std::format("objects/{}/{}.json", scene, kind);
    const auto buffer = io::read(filename);
    const auto j = nlohmann::json::parse(buffer);

    auto entity = _registry.create();

    tint tn;
    _registry.emplace<tint>(entity, std::move(tn));

    sprite s;
    s.pixmap = std::move(pixmappool->get(std::format("blobs/{}/{}.png", scene, kind));
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

    _proxies.emplace(std::move(name), std::make_shared<entityproxy>(entity, _registry));
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

  {
    auto view = _registry.view<animator, state>();

    for (auto entity : view) {
      auto& an = view.get<animator>(entity);
      auto& st = view.get<state>(entity);

      auto& tl = an.timelines[st.action];

      if (st.dirty) {
        st.current_frame = 0;
        st.tick = now;
        st.dirty = false;
        continue;
      }

      if (tl.frames.empty()) continue;

      const auto& frame = tl.frames[st.current_frame];
      if (frame.duration == 0 || now - st.tick < static_cast<uint64_t>(frame.duration)) {
        continue;
      }

      st.tick = now;

      if (st.current_frame + 1 >= tl.frames.size()) {
        if (!tl.next.empty()) {
          st.action = tl.next;
          st.dirty = true;
        } else {
          st.current_frame = 0;
        }
      } else {
        st.current_frame++;
      }
    }
  }

  {
    auto view = _registry.view<transform, animator, state, physics>();

    for (auto entity : view) {
      auto& tr = view.get<transform>(entity);
      auto& an = view.get<animator>(entity);
      auto& st = view.get<state>(entity);
      auto& ph = view.get<physics>(entity);

      if (!ph.enabled) {
        continue;
      }

      if (!ph.is_valid() && ph.dirty) {
        auto bodyDef = b2DefaultBodyDef();
        bodyDef.type = static_cast<b2BodyType>(ph.type);
        bodyDef.position = b2Vec2{tr.position.x, tr.position.y};

        bodyDef.userData = id_to_userdata(static_cast<uint64_t>(entity));

        ph.body = b2CreateBody(_world, &bodyDef);
      }

      if (ph.is_valid()) {
        b2Body_SetTransform(ph.body, b2Vec2{tr.position.x, tr.position.y}, b2Rot_identity);
      }

      if (ph.is_valid() && ph.dirty) {
        if (b2Shape_IsValid(ph.shape)) {
          b2DestroyShape(ph.shape, false);
        }

        const auto& opt = an[st.action].box;
        if (opt.has_value()) {
          const auto& box = opt.value();

          auto def = b2DefaultShapeDef();

          const float w = box.upperBound.x - box.lowerBound.x;
          const float h = box.upperBound.y - box.lowerBound.y;

          const float cx = box.lowerBound.x + w * .5f;
          const float cy = box.lowerBound.y + h * .5f;

          auto polygon = b2MakeOffsetBox(
            w * .5f,
            h * .5f,
            b2Vec2{cx, cy},
            b2Rot_identity);

          ph.shape = b2CreatePolygonShape(ph.body, &def, &polygon);
        }

        ph.dirty = false;
      }
    }
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
  static const auto w = static_cast<float>(_background->width());
  static const auto h = static_cast<float>(_background->height());

  _background->draw(.0f, .0f, w, h, .0f, .0f, w, h);

  const auto view = _registry.view<transform, sprite, animator, state>();

  for (auto entity : view) {
    auto& tr = view.get<transform>(entity);
    auto& sp = view.get<sprite>(entity);
    auto& an = view.get<animator>(entity);
    auto& st = view.get<state>(entity);

    const auto x = tr.position.x;
    const auto y = tr.position.y;

    const auto frame = an[st.action].frames[st.current_frame];

    sp.pixmap->draw(
      frame.quad.x + x, frame.quad.y + y,
      frame.quad.w, frame.quad.h,
      frame.quad.x, frame.quad.y,
      frame.quad.w, frame.quad.h
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

  const auto aabb = to_aabb(x0, y0, x1, y1);
  const auto filter = b2DefaultQueryFilter();

  b2World_OverlapAABB(_world, aabb, filter, _draw_callback, static_cast<SDL_Renderer*>(*_renderer));
#endif
}


void scene::set_onenter(std::function<void()>&& fn) {
  _onenter = std::move(fn);
}

void scene::set_onloop(sol::protected_function fn) {}
void scene::set_oncamera(sol::protected_function fn) {}
void scene::set_onleave(std::function<void()>&& fn) {}
void scene::set_ontouch(sol::protected_function fn) {}
void scene::set_onkeypress(sol::protected_function fn) {}
void scene::set_onkeyrelease(sol::protected_function fn) {}
void scene::set_ontext(sol::protected_function fn) {}
void scene::set_onmotion(sol::protected_function fn) {}

void scene::on_enter() const {
  if (auto fn = _onenter; fn) {
    fn();
  }
}

void scene::on_leave() const {
}

void scene::on_text(std::string_view text) const {
}

void scene::on_motion(float x, float y) const {
  static std::unordered_set<uint64_t> hits;
  hits.clear();
  hits.reserve(32);
  query(x, y, std::inserter(hits, hits.end()));

  for (const auto id : _hovering) {
    if (hits.contains(id)) continue;
    if (const auto entity = find(id)) {
      if (_registry.all_of<callbacks>(*entity)) {
        if (auto& callback = _registry.get<callbacks>(*entity); callback.on_unhover) {
          callback.on_unhover();
        }
      }
    }
  }

  for (const auto id : hits) {
    if (_hovering.contains(id)) continue;
    if (const auto entity = find(id)) {
      if (_registry.all_of<callbacks>(*entity)) {
        if (auto& callback = _registry.get<callbacks>(*entity); callback.on_hover) {
          callback.on_hover();
        }
      }
    }
  }

  _hovering.swap(hits);
}

void scene::on_touch(float x, float y) const {

}

void scene::on_key_press(int32_t code) const {

}

void scene::on_key_release(int32_t code) const {

}

std::optional<entt::entity> scene::find(uint64_t id) const {
  const auto entity = static_cast<entt::entity>(id);

  if (_registry.valid(entity)) {
    return entity;
  }

  return std::nullopt;
}








// scene::scene(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager)
//     : _name(name),
//       _j(j),
//       _scenemanager(std::move(scenemanager)) {
//   _objectmanager = _scenemanager->objectmanager();
//   _particlesystem = _scenemanager->particlesystem();
//   _resourcemanager = _scenemanager->resourcemanager();

//   const auto es = _j.value("effects", nlohmann::json::array());
//   _effects.reserve(es.size());
//   for (const auto& e : es) {
//     const auto basename = e.get<std::string_view>();
//     const auto f = std::format("blobs/{}/{}.ogg", _name, basename);
//     _effects.emplace_back(basename, _resourcemanager->soundmanager()->get(f));
//   }

//   const auto ps = _j.value("particles", nlohmann::json::array());
//   _particles.reserve(ps.size());
//   const auto factory = _particlesystem->factory();
//   for (const auto& i : ps) {
//     const auto particle = i["name"].get<std::string_view>();
//     const auto kind = i["kind"].get<std::string_view>();
//     const auto x = i["x"].get<float>();
//     const auto y = i["y"].get<float>();
//     const auto emitting = i.value("emitting", true);
//     _particles.emplace(particle, factory->create(kind, x, y, emitting));
//   }

//   const auto fs = _j.value("fonts", nlohmann::json::array());
//   for (const auto& i : fs) {
//     const auto fontname = i.get<std::string_view>();
//     _resourcemanager->fontfactory()->get(fontname);
//   }

//   const auto os = _j.value("objects", nlohmann::json::array());
//   _objects.reserve(os.size());
//   for (const auto& o : os) {
//     if (!o.is_object()) [[unlikely]] {
//       continue;
//     }

//     std::string key = o["name"].get<std::string>();
//     std::string kind = o["kind"].get<std::string>();

//     const float x = o.value("x", .0f);
//     const float y = o.value("y", .0f);

//     std::string action = o.value("action", std::string{});

//     auto object = _objectmanager->create(kind, _name, false);
//     object->set_placement(x, y);
//     if (!action.empty()) {
//       object->set_action(action);
//     }

//     _objects.emplace_back(std::move(key), std::move(object));
//   }
// }

// scene::~scene() noexcept {
//   auto objects = std::exchange(_objects, {});
//   for (const auto& [_, o] : objects) {
//     _objectmanager->remove(o);
//   }

//   auto effects = std::exchange(_effects, {});
//   for (const auto& [_, e] : effects) {
//     e->stop();
//   }

//   _particlesystem->clear();
// }

// void scene::update(float delta) {
//   if (auto fn = _onloop; fn) [[likely]] {
//     fn(delta);
//   }

//   if (auto fn = _oncamera; fn) [[likely]] {
//     _camera = fn(delta);
//   }
// }

// void scene::draw() const {
// }

// std::string_view scene::name() const noexcept {
//   return _name;
// }

// std::variant<
//   std::shared_ptr<object>,
//   std::shared_ptr<soundfx>,
//   std::shared_ptr<particleprops>
// > scene::get(std::string_view id, scenekind kind) const {
//   if (kind == scenekind::object) {
//     for (const auto& [key, object] : _objects) {
//       if (key == id) return object;
//     }
//   }

//   if (kind == scenekind::effect) {
//     for (const auto& [key, effect] : _effects) {
//       if (key == id) return effect;
//     }
//   }

//   if (kind == scenekind::particle) {
//     for (const auto& [key, batch] : _particles) {
//       if (key == id) return batch->props;
//     }
//   }

//   throw std::out_of_range(std::format("[scene] resource {} not found", id));
// }

// void scene::on_enter() const {
//   for (const auto& [_, o] : _objects) {
//     _objectmanager->manage(o);
//   }

//   for (const auto& [key, batch] : _particles) {
//     _particlesystem->add(batch);
//   }

//   if (auto fn = _onenter; fn) {
//     fn();
//   }
// }

// void scene::on_leave() const {
//   if (auto fn = _onleave; fn) {
//     fn();
//   }

//   _particlesystem->clear();

//   for (const auto& [_, o] : _objects) {
//     _objectmanager->remove(o);
//   }

//   for (const auto& [_, e] : _effects) {
//     e->stop();
//   }
// }

// void scene::on_touch(float x, float y) const {
//   if (auto fn = _ontouch; fn) {
//     fn(x, y);
//   }
// }

// void scene::on_key_press(int32_t code) const {
//   if (auto fn = _onkeypress; fn) {
//     fn(code);
//   }
// }

// void scene::on_key_release(int32_t code) const {
//   if (auto fn = _onkeyrelease; fn) {
//     fn(code);
//   }
// }

// void scene::on_text(std::string_view text) const {
//   if (auto fn = _ontext; fn) {
//     fn(text);
//   }
// }

// void scene::on_motion(float x, float y) const {
//   if (auto fn = _onmotion; fn) {
//     fn(x, y);
//   }
// }

// void scene::set_onenter(std::function<void()>&& fn) {
//   _onenter = std::move(fn);
// }

// void scene::set_onloop(sol::protected_function fn) {
//   _onloop = interop::wrap_fn<void(float)>(std::move(fn));
// }

// void scene::set_oncamera(sol::protected_function fn) {
//   _oncamera = interop::wrap_fn<quad(float)>(std::move(fn));
// }

// void scene::set_onleave(std::function<void()>&& fn) {
//   _onleave = std::move(fn);
// }

// void scene::set_ontouch(sol::protected_function fn) {
//   _ontouch = interop::wrap_fn<void(float, float)>(std::move(fn));
// }

// void scene::set_onkeypress(sol::protected_function fn) {
//   _onkeypress = interop::wrap_fn<void(int32_t)>(std::move(fn));
// }

// void scene::set_onkeyrelease(sol::protected_function fn) {
//   _onkeyrelease = interop::wrap_fn<void(int32_t)>(std::move(fn));
// }

// void scene::set_ontext(sol::protected_function fn) {
//   _ontext = interop::wrap_fn<void(std::string_view)>(std::move(fn));
// }

// void scene::set_onmotion(sol::protected_function fn) {
//   _onmotion = interop::wrap_fn<void(float, float)>(std::move(fn));
// }

// scenebackdrop::scenebackdrop(std::string_view name, const nlohmann::json& j, std::shared_ptr<scenemanager> scenemanager)
//     : scene(name, j, std::move(scenemanager)) {
//   _background = _resourcemanager->pixmappool()->get(std::format("blobs/{}/background.png", name));
// }

// void scenebackdrop::draw() const noexcept {
//   static const auto w = static_cast<float>(_background->width());
//   static const auto h = static_cast<float>(_background->height());

//   _background->draw(.0f, .0f, w, h, .0f, .0f, w, h);
// }
