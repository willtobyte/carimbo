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

  constexpr auto capacity = 128uz;
  _bodies.reserve(capacity);
  _objects.reserve(capacity);
  _dirties.reserve(capacity);
}

world::~world() noexcept {
  if (b2World_IsValid(_world)) {
    b2DestroyWorld(_world);
  }
}

static inline b2BodyId get_or_create_body(b2WorldId world, std::unordered_map<uint64_t, b2BodyId>& map, uint64_t id) noexcept {
  const auto it = map.find(id);
  if (it != map.end()) return it->second;

  auto def = b2DefaultBodyDef();
  def.type = b2_dynamicBody;
  def.gravityScale = 0.0f;
  def.fixedRotation = true;
  def.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id));
  const auto body = b2CreateBody(world, &def);
  map.emplace(id, body);
  return body;
}

static inline void assign_fixture(b2BodyId body, const geometry::rectangle& rectangle, bool isSensor = true) noexcept {
  const auto count = b2Body_GetShapeCount(body);
  if (count > 0) {
    std::vector<b2ShapeId> shapes(count);
    const int n = b2Body_GetShapes(body, shapes.data(), count);
    for (int i = 0; i < n; ++i) b2DestroyShape(shapes[i], false);
  }

  const auto hx = 0.5f * rectangle.width();
  const auto hy = 0.5f * rectangle.height();
  if (hx <= 0.0f || hy <= 0.0f) return;

  const auto cx = rectangle.x() + hx;
  const auto cy = rectangle.y() + hy;

  auto sd = b2DefaultShapeDef();
  sd.isSensor = isSensor;

  const auto box = b2MakeOffsetBox(hx, hy, b2Vec2{cx, cy}, b2MakeRot(0));
  b2CreatePolygonShape(body, &sd, &box);
}

void world::add(const std::shared_ptr<object>& object) {
  if (!object) [[unlikely]] return;

  const auto id = object->id();
  _objects[id] = object;

  const auto box = object->shape();
  if (!box.has_value()) [[unlikely]] {
    return;
  }

  const auto body = get_or_create_body(_world, _bodies, id);
  assign_fixture(body, *box);
}

void world::remove(uint64_t id) {
  const auto it = _bodies.find(id);
  if (it != _bodies.end()) [[likely]] {
    b2DestroyBody(it->second);
    _bodies.erase(it);
  }

  _objects.erase(id);
}

void world::update(float delta) noexcept {
  _dirties.clear();
  _dirties.reserve(_objects.size());

  for (auto it = _objects.begin(); it != _objects.end();) {
    const auto id = it->first;
    auto object = it->second.lock();
    if (!object) [[unlikely]] {
      const auto bit = _bodies.find(id);
      if (bit != _bodies.end()) {
        b2DestroyBody(bit->second);
        _bodies.erase(bit);
      }
      it = _objects.erase(it);
      continue;
    }

    if (!object->visible()) [[unlikely]] {
      const auto bit = _bodies.find(id);
      if (bit != _bodies.end()) {
        b2DestroyBody(bit->second);
        _bodies.erase(bit);
      }

      ++it;
      continue;
    }

    const auto shape = object->shape();
    if (!shape.has_value()) [[unlikely]] {
      const auto bit = _bodies.find(id);
      if (bit != _bodies.end()) {
        b2DestroyBody(bit->second);
        _bodies.erase(bit);
      }

      ++it;
      continue;
    }

    if (!object->dirty()) [[unlikely]] {
      ++it;
      continue;
    }

#ifndef NDEBUG
    std::println("[world] dirty object {} {}", object->kind(), id);
#endif

    const auto body = get_or_create_body(_world, _bodies, id);
    assign_fixture(body, *shape);
    _dirties.emplace_back(id);
    ++it;
  }

  b2World_Step(_world, std::max(.0f, delta), 4);

  // const auto events = b2World_GetContactEvents(_world);
  // for (auto i = events.beginCount; i-- > 0; ) {
  //   const auto& e = events.beginEvents[i];
  //   const auto a = b2Shape_GetBody(e.shapeIdA);
  //   const auto b = b2Shape_GetBody(e.shapeIdB);
  //   const auto ua = b2Body_GetUserData(a);
  //   const auto ub = b2Body_GetUserData(b);
  //   const auto first = static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(ua));
  //   const auto second = static_cast<uint64_t>(reinterpret_cast<std::uintptr_t>(ub));

  //   notify(first, second);
  // }
}

void world::draw() const noexcept {
#ifdef DEBUG
  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
  for (const auto& kv : _bodies) {
    const auto b = kv.second;
    const auto count = b2Body_GetShapeCount(b);
    if (count <= 0) continue;

    std::vector<b2ShapeId> shapes(count);
    const int n = b2Body_GetShapes(b, shapes.data(), count);
    for (int i = 0; i < n; ++i) {
      const b2AABB aabb = b2Shape_GetAABB(shapes[i]);
      SDL_FRect r;
      r.x = aabb.lowerBound.x;
      r.y = aabb.lowerBound.y;
      r.w = aabb.upperBound.x - r.x;
      r.h = aabb.upperBound.y - r.y;
      SDL_RenderRect(*_renderer, &r);
    }
  }
#endif
}

world::operator b2WorldId() const noexcept {
  return _world;
}

void world::notify(uint64_t first, uint64_t second) const {
  auto a = _objects.find(first)->second.lock();
  auto b = _objects.find(second)->second.lock();
  if (!a || !b) [[unlikely]] return;

  if (const auto* callback = find_ptr(a->_collision_mapping, b->kind())) (*callback)(a, b);
  if (const auto* callback = find_ptr(b->_collision_mapping, a->kind())) (*callback)(b, a);

  SDL_Event event{};
  event.type = static_cast<uint32_t>(input::event::type::collision);
  event.user.data1 = _envelopepool->acquire(collisionenvelope(a->id(), b->id())).release();
  SDL_PushEvent(&event);
}
