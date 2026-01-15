#pragma once

#include "common.hpp"

namespace physics {

enum class category : std::uint64_t {
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

enum class bodytype : uint8_t {
  staticbody = 0,
  kinematic,
  dynamic
};

struct raycast_hit {
  b2Vec2 point;
  b2Vec2 normal;
  float fraction;
  b2ShapeId shape;
  entt::entity entity;
};

[[nodiscard]] constexpr b2Vec2 to_b2(const vec2& v) noexcept {
  return {v.x, v.y};
}

[[nodiscard]] constexpr vec2 from_b2(b2Vec2 v) noexcept {
  return {v.x, v.y};
}

[[nodiscard]] constexpr b2AABB make_aabb(const vec2& center, float hx, float hy) noexcept {
  return {{center.x - hx, center.y - hy}, {center.x + hx, center.y + hy}};
}

[[nodiscard]] constexpr b2AABB make_aabb(float x, float y, float w, float h) noexcept {
  return {{x, y}, {x + w, y + h}};
}

[[nodiscard]] constexpr vec2 aabb_center(const b2AABB& aabb) noexcept {
  return {(aabb.lowerBound.x + aabb.upperBound.x) * 0.5f,
          (aabb.lowerBound.y + aabb.upperBound.y) * 0.5f};
}

[[nodiscard]] constexpr vec2 aabb_extents(const b2AABB& aabb) noexcept {
  return {(aabb.upperBound.x - aabb.lowerBound.x) * 0.5f,
          (aabb.upperBound.y - aabb.lowerBound.y) * 0.5f};
}

[[nodiscard]] constexpr b2Filter make_filter(category cat, category mask = category::all) noexcept {
  return {static_cast<uint64_t>(cat), static_cast<uint64_t>(mask), 0};
}

[[nodiscard]] constexpr b2QueryFilter make_query_filter(category cat = category::all, category mask = category::all) noexcept {
  return {static_cast<uint64_t>(cat), static_cast<uint64_t>(mask)};
}

[[nodiscard]] entt::entity entity_from(b2ShapeId shape) noexcept;
[[nodiscard]] bool valid_pair(b2ShapeId a, b2ShapeId b) noexcept;
[[nodiscard]] quad shape_aabb(b2ShapeId shape) noexcept;

class world;
class body;
class joint;

class world final {
public:
  world() noexcept = default;
  explicit world(float gravity_x, float gravity_y) noexcept;
  explicit world(const unmarshal::json& node) noexcept;
  ~world() noexcept;

  world(world&& other) noexcept;
  world& operator=(world&& other) noexcept;
  world(const world&) = delete;
  world& operator=(const world&) = delete;

  void step(float delta) noexcept;

  [[nodiscard]] b2WorldId id() const noexcept;
  [[nodiscard]] bool valid() const noexcept;
  [[nodiscard]] explicit operator bool() const noexcept;

  [[nodiscard]] b2SensorEvents sensor_events() const noexcept;

  [[nodiscard]] std::optional<raycast_hit> raycast_closest(const vec2& origin, const vec2& target, category mask = category::all) const noexcept;

  template <typename F>
  void raycast_all(const vec2& origin, const vec2& target, category mask, F&& callback) const noexcept {
    struct context { F* fn; };
    context ctx{&callback};
    const auto filter = make_query_filter(category::all, mask);
    const auto o = to_b2(origin);
    const auto t = to_b2(target);
    b2World_CastRay(_id, o, {t.x - o.x, t.y - o.y}, filter,
      [](b2ShapeId shape, b2Vec2 point, b2Vec2 normal, float fraction, void* userdata) -> float {
        auto* c = static_cast<context*>(userdata);
        const raycast_hit hit{point, normal, fraction, shape, entity_from(shape)};
        return (*c->fn)(hit) ? 1.0f : 0.0f;
      }, &ctx);
  }

  template <typename F>
  void overlap_aabb(const b2AABB& aabb, category mask, F&& callback) const noexcept {
    struct context { F* fn; };
    context ctx{&callback};
    const auto filter = make_query_filter(category::all, mask);
    b2World_OverlapAABB(_id, aabb, filter,
      [](b2ShapeId shape, void* userdata) -> bool {
        auto* c = static_cast<context*>(userdata);
        return (*c->fn)(shape, entity_from(shape));
      }, &ctx);
  }

  template <typename F>
  void overlap_circle(const vec2& center, float radius, category mask, F&& callback) const noexcept {
    struct context { F* fn; };
    context ctx{&callback};
    const auto o = to_b2(center);
    const auto proxy = b2MakeProxy(&o, 1, radius);
    const auto filter = make_query_filter(category::all, mask);
    b2World_OverlapShape(_id, &proxy, filter,
      [](b2ShapeId shape, void* userdata) -> bool {
        auto* c = static_cast<context*>(userdata);
        return (*c->fn)(shape, entity_from(shape));
      }, &ctx);
  }

private:
  b2WorldId _id{};
  float _accumulator{0.0f};
};

class body final {
public:
  body() noexcept = default;
  ~body() noexcept;

  body(body&& other) noexcept;
  body& operator=(body&& other) noexcept;
  body(const body&) = delete;
  body& operator=(const body&) = delete;

  [[nodiscard]] static body create(world& w, bodytype type, const vec2& position, entt::entity entity = entt::null) noexcept;
  [[nodiscard]] static body create_static(world& w, const vec2& position) noexcept;
  [[nodiscard]] static body create_static(world& w, const vec2& position, const vec2& half_extents) noexcept;

  void attach_sensor(float hx, float hy) noexcept;
  void attach_circle_sensor(float radius, const vec2& offset = {0, 0}) noexcept;
  void attach_capsule_sensor(const vec2& p1, const vec2& p2, float radius) noexcept;
  void attach_polygon_sensor(std::span<const b2Vec2> vertices) noexcept;

  void attach_box(float hx, float hy, float density = 1.0f, float friction = 0.6f) noexcept;
  void attach_circle(float radius, float density = 1.0f, float friction = 0.6f) noexcept;
  void attach_capsule(const vec2& p1, const vec2& p2, float radius, float density = 1.0f, float friction = 0.6f) noexcept;
  void attach_polygon(std::span<const b2Vec2> vertices, float density = 1.0f, float friction = 0.6f) noexcept;

  void detach_shape() noexcept;
  void destroy() noexcept;

  void set_transform(const vec2& position, float angle) noexcept;
  void set_position(const vec2& position) noexcept;
  void set_linear_velocity(const vec2& velocity) noexcept;
  void set_angular_velocity(float omega) noexcept;
  void set_gravity_scale(float scale) noexcept;
  void set_fixed_rotation(bool fixed) noexcept;
  void set_bullet(bool bullet) noexcept;
  void set_enabled(bool enabled) noexcept;
  void set_filter(category cat, category mask = category::all) noexcept;

  void apply_force(const vec2& force, const vec2& point) noexcept;
  void apply_force_center(const vec2& force) noexcept;
  void apply_impulse(const vec2& impulse, const vec2& point) noexcept;
  void apply_impulse_center(const vec2& impulse) noexcept;
  void apply_torque(float torque) noexcept;

  [[nodiscard]] vec2 position() const noexcept;
  [[nodiscard]] float angle() const noexcept;
  [[nodiscard]] vec2 linear_velocity() const noexcept;
  [[nodiscard]] float angular_velocity() const noexcept;
  [[nodiscard]] bool valid() const noexcept;
  [[nodiscard]] bool has_shape() const noexcept;
  [[nodiscard]] entt::entity entity() const noexcept;
  [[nodiscard]] quad shape_aabb() const noexcept;
  [[nodiscard]] explicit operator bool() const noexcept;

  [[nodiscard]] b2BodyId id() const noexcept;
  [[nodiscard]] b2ShapeId shape_id() const noexcept;

private:
  b2BodyId _body{};
  b2ShapeId _shape{};
};

class joint final {
public:
  joint() noexcept = default;
  ~joint() noexcept;

  joint(joint&& other) noexcept;
  joint& operator=(joint&& other) noexcept;
  joint(const joint&) = delete;
  joint& operator=(const joint&) = delete;

  [[nodiscard]] static joint distance(world& w, body& a, body& b, const vec2& anchor_a, const vec2& anchor_b, float length = -1.0f) noexcept;
  [[nodiscard]] static joint revolute(world& w, body& a, body& b, const vec2& anchor) noexcept;
  [[nodiscard]] static joint prismatic(world& w, body& a, body& b, const vec2& anchor, const vec2& axis) noexcept;
  [[nodiscard]] static joint motor(world& w, body& a, body& b, float max_force = 1000.0f, float max_torque = 1000.0f) noexcept;
  [[nodiscard]] static joint weld(world& w, body& a, body& b, const vec2& anchor) noexcept;
  [[nodiscard]] static joint wheel(world& w, body& chassis, body& whl, const vec2& anchor, const vec2& axis) noexcept;
  [[nodiscard]] static joint mouse(world& w, body& b, const vec2& target, float max_force) noexcept;

  void set_target(const vec2& target) noexcept;
  void destroy() noexcept;

  [[nodiscard]] bool valid() const noexcept;
  [[nodiscard]] explicit operator bool() const noexcept;

private:
  b2JointId _id{};
  b2BodyId _ground{};
};

}
