#pragma once

#include "common.hpp"

enum class collisioncategory : std::uint64_t {
  none = 0ull,
  player = 1ull << 0,
  enemy = 1ull << 1,
  projectile = 1ull << 2,
  terrain = 1ull << 3,
  trigger = 1ull << 4,
  collectible = 1ull << 5,
  interface = 1ull << 6,
  all = ~0ull
};

namespace physics {

inline void destroy_shape(b2ShapeId& shape) noexcept {
  if (!b2Shape_IsValid(shape)) [[unlikely]] return;
  b2DestroyShape(shape, false);
  shape = b2ShapeId{};
}

inline void destroy_body(b2BodyId& body) noexcept {
  if (!b2Body_IsValid(body)) [[unlikely]] return;
  b2DestroyBody(body);
  body = b2BodyId{};
}

inline void destroy(b2ShapeId& shape, b2BodyId& body) noexcept {
  destroy_shape(shape);
  destroy_body(body);
}

[[nodiscard]] inline entt::entity entity_from(b2ShapeId shape) noexcept {
  const auto body = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(body);
  return static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(data));
}

[[nodiscard]] inline bool valid_pair(b2ShapeId a, b2ShapeId b) noexcept {
  return b2Shape_IsValid(a) & b2Shape_IsValid(b);
}

[[nodiscard]] inline b2ShapeId make_sensor(b2BodyId body, float hx, float hy) noexcept {
  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  return b2CreatePolygonShape(body, &def, &poly);
}

[[nodiscard]] inline b2BodyId make_body(b2WorldId world, b2BodyType type, b2Vec2 position, b2Rot rotation, void* userdata = nullptr) noexcept {
  auto def = b2DefaultBodyDef();
  def.type = type;
  def.position = position;
  def.rotation = rotation;
  def.userData = userdata;
  return b2CreateBody(world, &def);
}

[[nodiscard]] inline b2BodyId make_static_body(b2WorldId world, b2Vec2 position) noexcept {
  auto def = b2DefaultBodyDef();
  def.type = b2_staticBody;
  def.position = position;
  return b2CreateBody(world, &def);
}

inline void destroy_world(b2WorldId& world) noexcept {
  if (!b2World_IsValid(world)) [[unlikely]] return;
  b2DestroyWorld(world);
  world = b2WorldId{};
}

}
