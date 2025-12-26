#include "scene.hpp"

#include "entityproxy.hpp"
#include "geometry.hpp"
#include "io.hpp"
#include "particlesystem.hpp"
#include "soundfx.hpp"

scene::scene(std::string_view name, unmarshal::document& document, std::shared_ptr<::scenemanager> scenemanager, sol::environment environment) {
  _renderer = scenemanager->renderer();
  _timermanager = std::make_shared<::timermanager>();

  _hits.reserve(64);

  SDL_GetRenderOutputSize(*_renderer, &_viewport_width, &_viewport_height);

  if (unmarshal::contains(document, "physics")) {
    auto def = b2DefaultWorldDef();
    if (auto physics = unmarshal::find_object(document, "physics")) {
      if (auto gravity = unmarshal::find_object(*physics, "gravity")) {
        def.gravity = b2Vec2{
          unmarshal::value_or(*gravity, "x", .0f),
          unmarshal::value_or(*gravity, "y", .0f)
        };
      }
    }

    _world.emplace(b2CreateWorld(&def));

    _physicssystem.emplace(_registry);
  }

  const auto& resourcemanager = scenemanager->resourcemanager();
  const auto& soundmanager = resourcemanager->soundmanager();
  const auto& pixmappool = resourcemanager->pixmappool();
  const auto& fontfactory = resourcemanager->fontfactory();

  if (auto effects = unmarshal::find_array(document, "effects")) {
    for (auto element : *effects) {
      const auto effect = unmarshal::string(element);
      const auto path = std::format("blobs/{}/{}.ogg", name, effect);
      _effects.emplace(effect, soundmanager->get(path));
    }
  }

  if (auto layer = unmarshal::find_object(document, "layer")) {
    const auto type = unmarshal::get<std::string_view>(*layer, "type");

    if (type == "tilemap")
      _layer.emplace<tilemap>(unmarshal::get<std::string_view>(*layer, "content"), resourcemanager);

    if (type == "background")
      _layer = pixmappool->get(std::format("blobs/{}/background.png", name));
  }

  auto z = 0;
  if (auto objects = unmarshal::find_array(document, "objects")) {
    for (auto element : *objects) {
      auto object = unmarshal::get<unmarshal::object>(element);
      const auto oname = unmarshal::get<std::string_view>(object, "name");
      const auto kind = unmarshal::get<std::string_view>(object, "kind");
      const auto action = _resolve(unmarshal::value_or(object, "action", std::string_view{}));

      const auto x = unmarshal::value_or(object, "x", .0f);
      const auto y = unmarshal::value_or(object, "y", .0f);

      const auto ofn = std::format("objects/{}/{}.json", name, kind);
      auto json = unmarshal::parse(io::read(ofn)); auto& dobject = *json;

      const auto entity = _registry.create();

      auto at = std::make_shared<atlas>();
      for (auto field : dobject["timelines"].get_object()) {
        at->timelines.emplace(_resolve(unmarshal::key(field)), unmarshal::make<timeline>(field.value()));
      }

      _registry.emplace<std::shared_ptr<const atlas>>(entity, std::move(at));

      metadata md{
        .kind = _resolve(kind)
      };
      _registry.emplace<metadata>(entity, std::move(md));

      _registry.emplace<tint>(entity);

      sprite sp{
        .pixmap = pixmappool->get(std::format("blobs/{}/{}.png", name, kind))
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

      if (_world) {
        _registry.emplace<physics>(entity);
      }

      renderable rd{
        .z = z++
      };
      _registry.emplace<renderable>(entity, std::move(rd));

      const auto lfn = std::format("objects/{}/{}.lua", name, kind);

      const auto proxy = std::make_shared<entityproxy>(entity, _registry);
      _proxies.emplace(std::move(oname), proxy);

      callbacks c {
        .self = proxy
      };
      _registry.emplace<callbacks>(entity, std::move(c));

      if (io::exists(lfn)) {
        sol::state_view lua(environment.lua_state());
        sol::environment env(lua, sol::create, environment);
        env["self"] = proxy;

        const auto buffer = io::read(lfn);
        std::string_view source{reinterpret_cast<const char*>(buffer.data()), buffer.size()};

        const auto result = lua.load(source, std::format("@{}", lfn));
        verify(result);

        auto function = result.get<sol::protected_function>();
        sol::set_environment(env, function);

        const auto exec = function();
        verify(exec);

        auto module = exec.get<sol::table>();

        scriptable sc;
        sc.environment = env;

        if (auto fn = module["on_begin"].get<sol::protected_function>(); fn.valid()) {
          sc.on_begin = std::move(fn);
        }

        if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
          sc.on_loop = std::move(fn);
        }

        if (auto fn = module["on_end"].get<sol::protected_function>(); fn.valid()) {
          sc.on_end = std::move(fn);
        }

        _registry.emplace<scriptable>(entity, std::move(sc));
      }
    }

    _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
      return lhs.z < rhs.z;
    });
  }

  if (auto particles = unmarshal::find_array(document, "particles")) {
    _particlesystem.emplace(scenemanager->resourcemanager());

    const auto factory = _particlesystem->factory();
    for (auto element : *particles) {
      auto object = unmarshal::get<unmarshal::object>(element);
      const auto pname = unmarshal::get<std::string_view>(object, "name");
      const auto kind = unmarshal::get<std::string_view>(object, "kind");
      const auto px = unmarshal::get<float>(object, "x");
      const auto py = unmarshal::get<float>(object, "y");
      const auto active = unmarshal::value_or(object, "active", true);
      const auto batch = factory->create(kind, px, py, active);
      _particles.emplace(pname, batch);
      _particlesystem->add(batch);
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
  if (!_world) {
    return;
  }

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

  if (b2World_IsValid(*_world)) {
    b2DestroyWorld(*_world);
  }
}

void scene::update(float delta) {
  const auto now = SDL_GetTicks();

  if (auto* layer = std::get_if<tilemap>(&_layer)) {
    const auto camera = _oncamera.call<vec2>(delta);
    layer->set_viewport({
        camera.x, camera.y,
        static_cast<float>(_viewport_width),
        static_cast<float>(_viewport_height)});
    layer->update(delta);
  }

  _animationsystem.update(now);

  if (_physicssystem) {
    _physicssystem->update(*_world, delta);
  }

  _scriptsystem.update(delta);

  if (_particlesystem) {
    _particlesystem->update(delta);
  }

  _onloop(delta);
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
  std::visit([](auto&& argument) {
    using T = std::decay_t<decltype(argument)>;
    if constexpr (std::is_same_v<T, std::shared_ptr<pixmap>>) {
      const auto w = static_cast<float>(argument->width());
      const auto h = static_cast<float>(argument->height());
      argument->draw(.0f, .0f, w, h, .0f, .0f, w, h);
    } else if constexpr (std::is_same_v<T, tilemap>) {
      argument.draw();
    }
  }, _layer);

  _rendersystem.draw();

  if (_particlesystem) {
    _particlesystem->draw();
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

  if (_world) {
    b2World_OverlapAABB(*_world, aabb, filter, _draw_callback, static_cast<SDL_Renderer*>(*_renderer));
  }
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

void scene::set_oncamera(sol::protected_function&& fn) {
  _oncamera = std::move(fn);
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
    _ontouch(x, y);
    return;
  }

  for (const auto entity : _hits) {
    if (const auto* c = _registry.try_get<callbacks>(entity)) {
      c->on_touch(c->self, x, y);
    }
  }
}

void scene::on_motion(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  for (const auto entity : _hovering) {
    if (_hits.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity)) {
      c->on_unhover(c->self);
    }
  }

  for (const auto entity : _hits) {
    if (_hovering.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity)) {
      c->on_hover(c->self);
    }
  }

  _hovering.swap(_hits);

  _onmotion(x, y);
}

void scene::on_key_press(int32_t code) {
  _onkeypress(code);
}

void scene::on_key_release(int32_t code) {
  _onkeyrelease(code);
}

void scene::on_text(std::string_view text) {
  _ontext(text);
}

std::shared_ptr<timermanager> scene::timermanager() const noexcept {
  return _timermanager;
}
