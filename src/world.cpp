#include "world.hpp"

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
  def.gravity = b2Vec2{.0f, .0f};
  _world = b2CreateWorld(&def);
}

world::~world() noexcept {
  if (b2World_IsValid(_world)) {
    b2DestroyWorld(_world);
  }
}

void world::add(const std::shared_ptr<object>& object) {
  // if (!object) [[unlikely]] return;
  // object->_world = shared_from_this();
}

void world::remove(uint64_t id) {
}

void world::update(float delta) noexcept {
  b2World_Step(_world, std::max(.0f, delta), 4);

  const auto events = b2World_GetSensorEvents(_world);
  for (auto i = events.beginCount; i-- > 0; ) {
    const auto& e = events.beginEvents[i];

    if (!b2Shape_IsValid(e.sensorShapeId) || !b2Shape_IsValid(e.visitorShapeId)) continue;

    const auto body_a = b2Shape_GetBody(e.sensorShapeId);
    const auto body_b = b2Shape_GetBody(e.visitorShapeId);
    const auto user_data_a = b2Body_GetUserData(body_a);
    const auto user_data_b = b2Body_GetUserData(body_b);

    if (!user_data_a || !user_data_b) continue;

    // Extract shared_ptr from userData
    auto* ptr_a = static_cast<std::shared_ptr<object>*>(user_data_a);
    auto* ptr_b = static_cast<std::shared_ptr<object>*>(user_data_b);

    notify(*ptr_a, *ptr_b);
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

  auto fun = [](b2ShapeId shape, void* ctx) -> bool {
    const auto* self = static_cast<const world*>(ctx);
    const b2AABB aabb = b2Shape_GetAABB(shape);

    SDL_FRect r{
      aabb.lowerBound.x,
      aabb.lowerBound.y,
      aabb.upperBound.x - r.x,
      aabb.upperBound.y - r.y
    };

    SDL_RenderRect(*self->_renderer, &r);
    return true;
  };

  b2World_OverlapAABB(_world, aabb, filter, fun, const_cast<world*>(this));
#endif
}

world::operator b2WorldId() const noexcept {
  return _world;
}

void world::notify(const std::shared_ptr<object>& obj_a, const std::shared_ptr<object>& obj_b) const {
  if (!obj_a || !obj_b) [[unlikely]] return;

  if (const auto* callback = find_ptr(obj_a->_collision_mapping, obj_b->kind())) (*callback)(obj_a, obj_b);
  if (const auto* callback = find_ptr(obj_b->_collision_mapping, obj_a->kind())) (*callback)(obj_b, obj_a);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::collision);
  event.user.data1 = _envelopepool->acquire(collisionenvelope(obj_a->id(), obj_b->id())).release();
  SDL_PushEvent(&event);
}
