#include "scene.hpp"

#include "components.hpp"
#include "geometry.hpp"
#include "physics.hpp"
#include "pixmap.hpp"

scene::scene(std::string_view name, unmarshal::json node, std::shared_ptr<::scenemanager> scenemanager, sol::environment environment)
    : _name(name),
      _soundmanager(name),
      _particlesystem(scenemanager->renderer()),
      _objectmanager(_registry, scenemanager->renderer(), name, environment) {
  _renderer = scenemanager->renderer();

  _hits.reserve(64);
  _registry.ctx().emplace<interning>();
  _registry.ctx().emplace<scripting>(_registry);

  auto def = b2DefaultWorldDef();
  if (auto physics = node["physics"]) {
    if (auto gravity = physics["gravity"]) {
      def.gravity.x = gravity["x"].get<float>();
      def.gravity.y = gravity["y"].get<float>();
    }
  }

  _world = b2CreateWorld(&def);

  if (auto effects = node["effects"]) {
    effects.foreach([this](unmarshal::json node) {
      _soundmanager.add(node.get<std::string_view>());
    });
  }

  if (auto layer = node["layer"]) {
    const auto type = layer["type"].get<std::string_view>();

    if (type == "tilemap") {
      const auto content = layer["content"].get<std::string_view>();
      auto& tilemap = _layer.emplace<::tilemap>(content, _renderer);

      const auto tile_size = tilemap.tile_size();
      const auto half = tile_size * 0.5f;
      const auto width = tilemap.width();

      auto sdef = b2DefaultShapeDef();
      const auto poly = b2MakeBox(half, half);

      for (const auto& grid : tilemap.grids()) {
        if (!grid.collider) continue;

        const auto* tiles = grid.tiles.data();
        for (int32_t row = 0; row < tilemap.height(); ++row) {
          const auto row_offset = row * width;
          for (int32_t column = 0; column < width; ++column) {
            if (tiles[row_offset + column] == 0) continue;

            const auto position = b2Vec2{
                static_cast<float>(column) * tile_size + half,
                static_cast<float>(row) * tile_size + half};

            const auto body = physics::make_static_body(_world, position);
            b2CreatePolygonShape(body, &sdef, &poly);
          }
        }
      }
    }

    if (type == "background") {
      _layer = std::make_shared<pixmap>(_renderer, std::format("blobs/{}/background.png", name));

      const auto width = node["width"].get<float>();
      const auto height = node["height"].get<float>();
      _camera = quad{0, 0, width, height};
    }
  }

  if (auto objects = node["objects"]) {
    auto z = 0;
    objects.foreach([this, &z](unmarshal::json node) {
      _objectmanager.add(std::move(node), z++);
    });

    _objectmanager.sort();
  }

  if (auto particles = node["particles"]) {
    particles.foreach([this](unmarshal::json node) {
      _particlesystem.add(std::move(node));
    });
  }
}

scene::~scene() noexcept {
  for (auto&& [entity, r] : _registry.view<rigidbody>().each()) {
    physics::destroy(r.shape, r.body);
  }

  physics::destroy_world(_world);
}

void scene::update(float delta) {
  const auto now = SDL_GetTicks();

  if (auto* layer = std::get_if<tilemap>(&_layer)) {
    _camera = _oncamera.call<quad>(delta);

    layer->set_viewport(_camera);

    layer->update(delta);
  }

  _animationsystem.update(now);

  _physicssystem.update(_world, delta);

  _soundmanager.update(delta);

  _particlesystem.update(delta);

  _scriptsystem.update(delta);

  _onloop(delta);
}

#ifdef DEBUG
[[nodiscard]] static bool _draw_callback(const b2ShapeId shape, void* const ctx) {
  const auto [renderer, camera] = *static_cast<std::pair<SDL_Renderer*, const quad*>*>(ctx);
  const auto aabb = b2Shape_GetAABB(shape);

  const auto r = SDL_FRect{
    aabb.lowerBound.x - camera->x,
    aabb.lowerBound.y - camera->y,
    aabb.upperBound.x - aabb.lowerBound.x,
    aabb.upperBound.y - aabb.lowerBound.y
  };

  SDL_RenderRect(renderer, &r);
  return true;
}
#endif

void scene::draw() const noexcept {
  std::visit([this](auto&& argument) {
    using T = std::decay_t<decltype(argument)>;
    if constexpr (std::is_same_v<T, std::shared_ptr<pixmap>>) {
      argument->draw(_camera.x, _camera.y, _camera.w, _camera.h, .0f, .0f, _camera.w, _camera.h);
    } else if constexpr (std::is_same_v<T, tilemap>) {
      argument.draw();
    }
  }, _layer);

  _rendersystem.draw();

  _particlesystem.draw();

#ifdef DEBUG
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);

  b2AABB aabb{{_camera.x, _camera.y}, {_camera.x + _camera.w, _camera.y + _camera.h}};

  auto filter = b2DefaultQueryFilter();
  auto context = std::pair{static_cast<SDL_Renderer*>(*_renderer), &_camera};
  b2World_OverlapAABB(_world, aabb, filter, _draw_callback, &context);

  SDL_SetRenderDrawColor(*_renderer, 0, 0, 0, 0);
#endif
}

void scene::populate(sol::table& pool) const {
  _soundmanager.populate(pool);
  _particlesystem.populate(pool);
  _objectmanager.populate(pool);
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

  _soundmanager.stop();

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

  for (auto&& [entity, t] : _registry.view<tickable>().each()) {
    t.on_tick(tick);
  }
}
