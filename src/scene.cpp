#include "scene.hpp"

#include "geometry.hpp"
#include "object.hpp"
#include "objectmanager.hpp"
#include "particlesystem.hpp"
#include "pixmap.hpp"
#include "scenemanager.hpp"
#include "soundfx.hpp"
#include "tilemap.hpp"

scene::scene(std::string_view scene, const nlohmann::json& json, std::shared_ptr<scenemanager> scenemanager) {
  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);

  const auto pixmappool = scenemanager->resourcemanager()->pixmappool();

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

    _spritesheets.emplace(++_sprite_counter, pixmappool->get(std::format("blobs/{}/{}.png", scene, kind)));

    auto entity = _registry.create();

    tint tn;
    _registry.emplace<tint>(entity, std::move(tn));

    sprite s;
    s.id = _sprite_counter;
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
  {
    auto view = _registry.view<transform, animator, state, physics>();
    for (auto entity : view) {
      auto& tr = view.get<transform>(entity);
      auto& an = view.get<animator>(entity);
      auto& st = view.get<state>(entity);
      auto& ph = view.get<physics>(entity);

      if (!ph.enabled || !ph.dirty || !ph.is_valid()) {
        continue;
      }

      if (b2Shape_IsValid(ph.shape)) {
        b2DestroyShape(ph.shape, false);
      }

      const auto& box = an[st.action].box;
      auto sdef = b2DefaultShapeDef();
      auto polygon = b2MakeBox(
        (box.upperBound.x - box.lowerBound.x) * .5f,
        (box.upperBound.y - box.lowerBound.y) * .5f);

      ph.shape = b2CreatePolygonShape(ph.body, &sdef, &polygon);

      ph.dirty = false;
    }
  }
}

void scene::draw() const noexcept {
  const auto view = _registry.view<transform, tint, sprite, animator, state>();

  for (auto entity : view) {
    auto& tr = view.get<transform>(entity);
    auto& tn = view.get<tint>(entity);
    auto& sp = view.get<sprite>(entity);
    auto& an = view.get<animator>(entity);
    auto& st = view.get<state>(entity);

    const auto x = tr.position.x;
    const auto y = tr.position.y;

    const auto it = _spritesheets.find(sp.id);
    assert(it != _spritesheets.end() && "attempted to access non-existent spritesheet");

    const auto frame = an[st.action].frames[st.current_frame];

    it->second->draw(
      frame.quad.x + x, frame.quad.y + y,
      frame.quad.w, frame.quad.h,
      frame.quad.x, frame.quad.y,
      frame.quad.w, frame.quad.h
    );
    // const auto& frame = tl.frames[0];

    // std::println("id {}", sp.id);
    // const auto it = _spritesheets.find(sp.id);
    // assert(it != _spritesheets.end() && "key not found in _spritesheets");

    // it->second->draw(
    //   frame.quad.x, frame.quad.y, frame.quad.w, frame.quad.h,
    //   0, 0, frame.quad.w, frame.quad.h,
    //   tr.angle
    // );
  }
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
