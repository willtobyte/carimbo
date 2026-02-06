#include "physics.hpp"

#include "geometry.hpp"

using namespace physics;

entt::entity physics::entity_from(b2ShapeId shape) noexcept {
  const auto data = b2Shape_GetUserData(shape);
  return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
}

bool physics::valid_pair(b2ShapeId a, b2ShapeId b) noexcept {
  return b2Shape_IsValid(a) & b2Shape_IsValid(b);
}

world::world(const unmarshal::json& node) noexcept {
  auto gx = .0f, gy = .0f;
  if (auto p = node["physics"]) {
    if (auto g = p["gravity"]) {
      gx = g["x"].get(.0f);
      gy = g["y"].get(.0f);
    }
  }

  auto def = b2DefaultWorldDef();
  def.gravity = {gx, gy};
  _id = b2CreateWorld(&def);
}

world::~world() noexcept {
  if (b2World_IsValid(_id)) {
    b2DestroyWorld(_id);
  }
}

world::world(world&& other) noexcept
    : _id(other._id), _accumulator(other._accumulator) {
  other._id = b2WorldId{};
  other._accumulator = .0f;
}

world& world::operator=(world&& other) noexcept {
  if (this != &other) {
    if (b2World_IsValid(_id)) {
      b2DestroyWorld(_id);
    }

    _id = other._id;
    _accumulator = other._accumulator;
    other._id = b2WorldId{};
    other._accumulator = .0f;
  }

  return *this;
}

b2WorldId world::id() const noexcept {
  return _id;
}

body body::create(world& w, const bodydef& d) noexcept {
  body result;
  auto def = b2DefaultBodyDef();
  def.type = static_cast<b2BodyType>(d.type);
  def.position = to_b2(d.position);
  def.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(d.entity));
  result.id = b2CreateBody(w.id(), &def);

  if (d.box) {
    const auto poly = b2MakeBox(d.box->x, d.box->y);
    auto shape_def = b2DefaultShapeDef();
    shape_def.isSensor = d.sensor;
    shape_def.enableSensorEvents = d.sensor;
    shape_def.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(d.entity));
    result.shape = b2CreatePolygonShape(result.id, &shape_def, &poly);
    if (d.sensor) result.cache = {d.box->x, d.box->y};
  }

  return result;
}

void body::destroy() noexcept {
  detach();

  if (b2Body_IsValid(id)) {
    b2DestroyBody(id);
    id = b2BodyId{};
  }
}

void body::attach_sensor(float hx, float hy) noexcept {
  if (has_shape() && cache.hx == hx && cache.hy == hy) [[likely]] return;

  detach();

  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  def.userData = b2Body_GetUserData(id);
  shape = b2CreatePolygonShape(id, &def, &poly);
  cache = {hx, hy};
}

void body::detach() noexcept {
  if (b2Shape_IsValid(shape)) {
    b2DestroyShape(shape, false);
    shape = b2ShapeId{};
  }
}

void body::transform(const vec2& position, float angle) noexcept {
  b2Body_SetTransform(id, to_b2(position), b2MakeRot(angle));
}

bool body::has_shape() const noexcept {
  return b2Shape_IsValid(shape);
}
