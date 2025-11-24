#include "world.hpp"

#include "objectmanager.hpp"
#include "physics.hpp"
#include "renderer.hpp"

template <class Map, class Key>
[[nodiscard]] static inline const typename Map::mapped_type* find_ptr(
  const Map& m,
  const Key& k
) {
  const auto it = m.find(k);
  if (it == m.end()) return nullptr;
  return std::addressof(it->second);
}

world::world(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {
  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);
  _collisions.reserve(64);
}

world::~world() {
  if (b2World_IsValid(_world)) {
    b2DestroyWorld(_world);
  }
}

void world::update(const float delta) {
  _accumulator += std::min(delta, 0.1f);

  while (_accumulator >= FIXED_TIMESTEP) {
    b2World_Step(_world, FIXED_TIMESTEP, WORLD_SUBSTEPS);
    _accumulator -= FIXED_TIMESTEP;
  }

  const auto events = b2World_GetSensorEvents(_world);

  for (auto i = events.beginCount; i-- > 0; ) {
    const auto& e = events.beginEvents[i];

    const auto body_a = b2Shape_GetBody(e.sensorShapeId);
    const auto body_b = b2Shape_GetBody(e.visitorShapeId);

    if (!b2Body_IsValid(body_a) || !b2Body_IsValid(body_b)) [[unlikely]] continue;

    const auto user_data_a = b2Body_GetUserData(body_a);
    const auto user_data_b = b2Body_GetUserData(body_b);

    if (!user_data_a || !user_data_b) [[unlikely]] continue;

    const auto id_a = userdata_to_id(user_data_a);
    const auto id_b = userdata_to_id(user_data_b);

    const auto pair = std::minmax(id_a, id_b);
    const auto [it, inserted] = _collisions.insert(pair);

    if (inserted) {
      notify(id_a, id_b);
    }
  }

  for (auto i = events.endCount; i-- > 0; ) {
    const auto& e = events.endEvents[i];

    const auto body_a = b2Shape_GetBody(e.sensorShapeId);
    const auto body_b = b2Shape_GetBody(e.visitorShapeId);

    if (!b2Body_IsValid(body_a) || !b2Body_IsValid(body_b)) [[unlikely]] continue;

    const auto user_data_a = b2Body_GetUserData(body_a);
    const auto user_data_b = b2Body_GetUserData(body_b);

    if (!user_data_a || !user_data_b) [[unlikely]] continue;

    const auto id_a = userdata_to_id(user_data_a);
    const auto id_b = userdata_to_id(user_data_b);

    const auto pair = std::minmax(id_a, id_b);
    _collisions.erase(pair);
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

void world::draw() const {
#ifdef DEBUG
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);

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

world::operator b2WorldId() const noexcept {
  return _world;
}

bool world::collides(const std::shared_ptr<object>& a, const std::shared_ptr<object>& b) const {
  const auto id_a = a->id();
  const auto id_b = b->id();
  const auto pair = std::minmax(id_a, id_b);
  return _collisions.contains(pair);
}

void world::notify(const uint64_t id_a, const uint64_t id_b) const {
  const auto objectmanager = _objectmanager.lock();
  if (!objectmanager) [[unlikely]] return;

  const auto object_a = objectmanager->find(id_a);
  const auto object_b = objectmanager->find(id_b);

  if (!object_a || !object_b) [[unlikely]] return;

  if (const auto* const callback_ptr = find_ptr(object_a->_collision_mapping, object_b->kind())) {
    auto callback = *callback_ptr;
    callback(object_a, object_b);
  }

  if (const auto* const callback_ptr = find_ptr(object_b->_collision_mapping, object_a->kind())) {
    auto callback = *callback_ptr;
    callback(object_b, object_a);
  }
}

void world::set_objectmanager(std::weak_ptr<objectmanager> objectmanager) {
  _objectmanager = std::move(objectmanager);
}
