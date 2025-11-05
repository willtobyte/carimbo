#include "world.hpp"
#include "objectmanager.hpp"
#include "physics.hpp"

using namespace framework;

template <class Map>
static inline const typename Map::mapped_type* find_ptr(
  const Map& m,
  const typename Map::key_type& k
) noexcept {
  const auto it = m.find(k);
  if (it == m.end()) return nullptr;
  return std::addressof(it->second);
}

world::world(std::shared_ptr<graphics::renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {
  auto def = b2DefaultWorldDef();
  def.gravity = b2Vec2{0.0f, 0.0f};
  _world = b2CreateWorld(&def);
}

world::~world() noexcept {
  if (b2World_IsValid(_world)) {
    b2DestroyWorld(_world);
  }
}

void world::update(float delta) noexcept {
  _accumulator += std::min(delta, 0.1f);

  while (_accumulator >= FIXED_TIMESTEP) {
    b2World_Step(_world, FIXED_TIMESTEP, WORLD_SUBSTEPS);

    std::unordered_set<std::pair<uint64_t, uint64_t>, boost::hash<std::pair<uint64_t, uint64_t>>> collisions;
    collisions.reserve(8);

    const auto events = b2World_GetSensorEvents(_world);
    for (auto i = events.beginCount; i-- > 0; ) {
      const auto& e = events.beginEvents[i];

      const auto body_a = b2Shape_GetBody(e.sensorShapeId);
      const auto body_b = b2Shape_GetBody(e.visitorShapeId);

      const auto user_data_a = b2Body_GetUserData(body_a);
      const auto user_data_b = b2Body_GetUserData(body_b);

      if (!user_data_a || !user_data_b) [[unlikely]] continue;

      const auto id_a = physics::userdata_to_id(user_data_a);
      const auto id_b = physics::userdata_to_id(user_data_b);

      auto pair = std::minmax(id_a, id_b);
      if (collisions.insert(pair).second) {
        notify(id_a, id_b);
      }
    }

    _accumulator -= FIXED_TIMESTEP;
  }
}

void world::draw() const noexcept {
#ifdef DEBUG
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);

  int w = 0, h = 0;
  SDL_GetRenderOutputSize(*_renderer, &w, &h);

  const auto x0 = 0.0f;
  const auto y0 = 0.0f;
  const auto x1 = static_cast<float>(w);
  const auto y1 = static_cast<float>(h);

  const auto aabb = to_aabb(x0, y0, x1, y1);
  auto filter = b2DefaultQueryFilter();

  auto fn = [](b2ShapeId shape, void* ctx) -> bool {
    const auto* self = static_cast<const world*>(ctx);
    const b2AABB aabb = b2Shape_GetAABB(shape);

    SDL_FRect r{
      aabb.lowerBound.x,
      aabb.lowerBound.y,
      aabb.upperBound.x - aabb.lowerBound.x,
      aabb.upperBound.y - aabb.lowerBound.y
    };

    SDL_RenderRect(*self->_renderer, &r);
    return true;
  };

  b2World_OverlapAABB(_world, aabb, filter, fn, const_cast<world*>(this));
#endif
}

world::operator b2WorldId() const noexcept {
  return _world;
}

void world::notify(uint64_t id_a, uint64_t id_b) const {
  auto objectmanager = _objectmanager.lock();
  if (!objectmanager) [[unlikely]] return;

  auto object_a = objectmanager->find(id_a);
  auto object_b = objectmanager->find(id_b);

  if (!object_a || !object_b) [[unlikely]] return;

  if (const auto* callback = find_ptr(object_a->_collision_mapping, object_b->kind())) (*callback)(object_a, object_b);
  if (const auto* callback = find_ptr(object_b->_collision_mapping, object_a->kind())) (*callback)(object_b, object_a);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::collision);
  event.user.data1 = _envelopepool->acquire(collisionenvelope(id_a, id_b)).release();
  SDL_PushEvent(&event);
}

void world::set_objectmanager(std::weak_ptr<objectmanager> objectmanager) noexcept {
  _objectmanager = std::move(objectmanager);
}
