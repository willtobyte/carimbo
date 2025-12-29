#include "scene.hpp"

#include "geometry.hpp"

scene::scene(std::string_view name, unmarshal::document& document, std::shared_ptr<::scenemanager> scenemanager, sol::environment environment)
    :
      _effects(scenemanager->resourcemanager()->soundmanager(), name),
      _particles(scenemanager->resourcemanager()),
      _objects(_registry, scenemanager->resourcemanager()->pixmappool(), name, environment) {
  _renderer = scenemanager->renderer();
  _timermanager = std::make_shared<::timermanager>();

  _hits.reserve(64);

  auto def = b2DefaultWorldDef();
  if (auto physics = unmarshal::find<unmarshal::object>(document, "physics")) {
    if (auto gravity = unmarshal::find<unmarshal::object>(*physics, "gravity")) {
      def.gravity = b2Vec2{
        unmarshal::value_or(*gravity, "x", .0f),
        unmarshal::value_or(*gravity, "y", .0f)
      };
    }
  }

  _world = b2CreateWorld(&def);

  const auto& resourcemanager = scenemanager->resourcemanager();
  const auto& pixmappool = resourcemanager->pixmappool();
  const auto& fontfactory = resourcemanager->fontfactory();

  if (auto array = unmarshal::find<unmarshal::array>(document, "effects")) {
    for (auto element : *array) {
      _effects.add(unmarshal::string(element));
    }
  }

  if (auto layer = unmarshal::find<unmarshal::object>(document, "layer")) {
    const auto type = unmarshal::get<std::string_view>(*layer, "type");

    if (type == "tilemap") {
      auto& tilemap = _layer.emplace<::tilemap>(unmarshal::get<std::string_view>(*layer, "content"), resourcemanager);

      const auto tile_size = tilemap.tile_size();
      const auto half = tile_size * 0.5f;
      const auto width = tilemap.width();

      auto bdef = b2DefaultBodyDef();
      bdef.type = b2_staticBody;

      auto shape = b2DefaultShapeDef();
      const auto poly = b2MakeBox(half, half);

      for (const auto& grid : tilemap.grids()) {
        if (!grid.collider) continue;

        const auto* tiles = grid.tiles.data();
        for (int32_t row = 0; row < tilemap.height(); ++row) {
          const auto row_offset = row * width;
          for (int32_t column = 0; column < width; ++column) {
            if (tiles[row_offset + column] == 0) continue;

            bdef.position = {
              static_cast<float>(column) * tile_size + half,
              static_cast<float>(row) * tile_size + half
            };

            const auto body = b2CreateBody(_world, &bdef);
            b2CreatePolygonShape(body, &shape, &poly);
          }
        }
      }
    }

    if (type == "background") {
      _layer = pixmappool->get(std::format("blobs/{}/background.png", name));

      _camera = quad{0, 0, unmarshal::get<float>(document, "width"), unmarshal::get<float>(document, "height")};
    }
  }

  if (auto array = unmarshal::find<unmarshal::array>(document, "objects")) {
    auto z = 0;
    for (auto element : *array) {
      auto object = unmarshal::get<unmarshal::object>(element);
      _objects.add(object, z++);
    }

    _objects.sort();
  }

  if (auto array = unmarshal::find<unmarshal::array>(document, "particles")) {
    for (auto element : *array) {
      auto particle = unmarshal::get<unmarshal::object>(element);
      _particles.add(particle);
    }
  }

  if (auto array = unmarshal::find<unmarshal::array>(document, "fonts")) {
    for (auto element : *array) {
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

void scene::update(float delta) {
  const auto now = SDL_GetTicks();

  if (auto* layer = std::get_if<tilemap>(&_layer)) {
    _camera = _oncamera.call<quad>(delta);

    layer->set_viewport(_camera);

    layer->update(delta);
  }

  _animationsystem.update(now);

  _physicssystem.update(_world, delta);

  _particles.update(delta);

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

  _particles.draw();

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
  _effects.populate(pool);
  _particles.populate(pool);
  _objects.populate(pool);
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

  _effects.stop();

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
      auto self = c->self.lock();
      assert(self && "self expired");
      c->on_touch(self, x, y);
    }
  }
}

void scene::on_motion(float x, float y) {
  _hits.clear();
  query(x, y, _hits);

  for (const auto entity : _hovering) {
    if (_hits.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity)) {
      auto self = c->self.lock();
      assert(self && "self expired");
      c->on_unhover(self);
    }
  }

  for (const auto entity : _hits) {
    if (_hovering.contains(entity)) continue;
    if (const auto* c = _registry.try_get<callbacks>(entity)) {
      auto self = c->self.lock();
      assert(self && "self expired");
      c->on_hover(self);
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

void scene::on_mail(uint64_t to, uint64_t from, std::string_view body) {
  const auto entity = static_cast<entt::entity>(to);
  const auto* c = _registry.try_get<callbacks>(entity);
  if (!c) [[unlikely]] return;

  auto self = c->self.lock();
  assert(self && "recipient expired");

  const auto fe = static_cast<entt::entity>(from);
  const auto* fc = _registry.try_get<callbacks>(fe);
  if (!fc) [[unlikely]] return;

  auto sender = fc->self.lock();
  assert(sender && "sender expired");

  c->on_mail(self, sender, body);
}

std::shared_ptr<timermanager> scene::timermanager() const noexcept {
  return _timermanager;
}
