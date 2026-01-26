#include "scene.hpp"

#include "components.hpp"
#include "fontpool.hpp"
#include "geometry.hpp"
#include "objectproxy.hpp"
#include "particlepool.hpp"
#include "physics.hpp"
#include "pixmap.hpp"

scene::scene(std::string_view name, unmarshal::json node, std::shared_ptr<::fontpool> fontpool, sol::environment environment)
    : _name(name),
      _world(node),
      _environment(std::move(environment)),
      _soundpool(name),
      _objectpool(_registry, _world, name, _environment) {
  (void)fontpool;
  _view = _registry.view<tickable>();
  _hits.reserve(16);
  _hovering.reserve(16);

  _registry.ctx().emplace<interning>();
  _registry.ctx().emplace<scripting>(_registry);
  _registry.ctx().emplace<physics::world*>(&_world);
  _registry.ctx().emplace<renderstate>();

  _registry.on_destroy<physics::body>().connect<[](entt::registry& registry, entt::entity entity) {
    registry.get<physics::body>(entity).destroy();
  }>();

  if (auto sounds = node["sounds"]) {
    sounds.foreach([this](unmarshal::json node) {
      _soundpool.add(node.get<std::string_view>());
    });
  }

  if (auto fonts = node["fonts"]) {
    fonts.foreach([&fontpool](unmarshal::json node) {
      fontpool->get(node.get<std::string_view>());
    });
  }

  if (auto layer = node["layer"]) {
    const auto type = layer["type"].get<std::string_view>();

    if (type == "tilemap") {
      auto content = layer["content"].get<std::string_view>();
      _layer.emplace<::tilemap>(content, _world);
    }

    else {
      _layer = std::make_shared<pixmap>(std::format("blobs/{}/background.png", name));

      const auto width = node["width"].get<float>();
      const auto height = node["height"].get<float>();
      _camera = quad{0, 0, width, height};
    }
  }

  if (auto objects = node["objects"]) {
    auto z = 0;
    objects.foreach([this, &z](unmarshal::json node) {
      const auto type = node["type"].get<std::string_view>("object");
      if (type == "particle") {
        _particlepool.add(std::move(node), z++);
      } else {
        _objectpool.add(std::move(node), z++);
      }
    });

    _objectpool.sort();
  }
}

void scene::update(float delta) {
  const auto now = SDL_GetTicks();

  if (auto* layer = std::get_if<tilemap>(&_layer)) {
    _camera = _oncamera.call<quad>(delta);

    layer->set_viewport(_camera);

    layer->update(delta);
  }

  _animationsystem.update(now);

  _physicssystem.update(delta);

  _soundpool.update(delta);

  _particlepool.update(delta);

  _velocitysystem.update(delta);

  _scriptsystem.update(delta);

  _rendersystem.update();

  auto& state = _registry.ctx().get<renderstate>();
  state.flush(_registry);

  _onloop(delta);
}

void scene::draw() const noexcept {
  std::visit([this](auto&& argument) {
    using T = std::decay_t<decltype(argument)>;
    if constexpr (std::is_same_v<T, std::shared_ptr<pixmap>>) {
      argument->draw(_camera.x, _camera.y, _camera.w, _camera.h, .0f, .0f, _camera.w, _camera.h);
    } else if constexpr (std::is_same_v<T, tilemap>) {
      argument.draw();
    }
  }, _layer);

  for (auto&& [entity, rn] : _registry.view<renderable>().each()) {
    if (!rn.visible) [[unlikely]] continue;

    switch (rn.kind) {
    case renderablekind::sprite:
      _objectpool.draw(entity);
      break;

    case renderablekind::particle:
      _particlepool.draw(entity);
      break;
    }
  }

#ifdef DEBUG
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);

  _world.query_aabb(physics::aabb(_camera), physics::category::all, [this](b2ShapeId shape, entt::entity) {
    const auto box = b2Shape_GetAABB(shape);
    const SDL_FRect r{
      box.lowerBound.x - _camera.x,
      box.lowerBound.y - _camera.y,
      box.upperBound.x - box.lowerBound.x,
      box.upperBound.y - box.lowerBound.y
    };

    SDL_RenderRect(renderer, &r);

    return true;
  });

  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
#endif
}

std::string_view scene::name() const noexcept {
  return _name;
}

void scene::populate(sol::table& pool) const {
  sol::state_view lua(pool.lua_state());

  lua.new_enum(
    "PhysicsCategory",
    "none", physics::category::none,
    "player", physics::category::player,
    "enemy", physics::category::enemy,
    "projectile", physics::category::projectile,
    "terrain", physics::category::terrain,
    "trigger", physics::category::trigger,
    "collectible", physics::category::collectible,
    "interface", physics::category::interface,
    "all", physics::category::all
  );

  const auto raycast = [&](const vec2& origin, float angle, float distance, std::optional<physics::category> mask) {
    boost::container::small_vector<std::shared_ptr<objectproxy>, 16> result;
    _world.raycast(origin, angle, distance, mask.value_or(physics::category::all), [&](entt::entity entity) {
      if (const auto* proxy = _registry.try_get<std::shared_ptr<objectproxy>>(entity)) {
        result.push_back(*proxy);
      }
    });

    return sol::as_table(std::move(result));
  };

  lua["world"] = lua.create_table_with(
    "raycast", sol::overload(
      [raycast](const vec2& origin, float angle, float distance, std::optional<physics::category> mask) {
        return raycast(origin, angle, distance, mask);
      },
      [raycast](float x, float y, float angle, float distance, std::optional<physics::category> mask) {
        return raycast({x, y}, angle, distance, mask);
      }
    )
  );

  _soundpool.populate(pool);
  _particlepool.populate(pool);
  _objectpool.populate(pool);
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

void scene::set_ontick(sol::protected_function&& fn) {
  _ontick = std::move(fn);
}

void scene::on_enter() {
  assert(_onenter && "on_enter callback must be set");
  _onenter();

  for (auto&& [entity, sc] : _registry.view<scriptable>().each()) {
    sc.on_spawn();
  }
}

void scene::on_leave() {
  for (auto&& [entity, sc] : _registry.view<scriptable>().each()) {
    sc.on_dispose();
  }

  _soundpool.stop();

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
    if (const auto* t = _registry.try_get<touchable>(entity)) {
      t->on_touch(x, y);
    }
  }
}

void scene::on_motion(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  for (const auto entity : _hovering) {
    if (_hits.contains(entity)) continue;
    if (const auto* h = _registry.try_get<hoverable>(entity)) {
      h->on_unhover();
    }
  }

  for (const auto entity : _hits) {
    if (_hovering.contains(entity)) continue;
    if (const auto* h = _registry.try_get<hoverable>(entity)) {
      h->on_hover();
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

void scene::on_tick(uint8_t tick) {
  _ontick(tick);

  for (auto&& [entity, t] : _view.each()) {
    t.on_tick(tick);
  }
}
