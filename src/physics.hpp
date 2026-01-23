#pragma once

#include "common.hpp"

namespace physics {

enum class category : std::uint32_t {
  none = 0u,
  player = 1u << 0,
  enemy = 1u << 1,
  projectile = 1u << 2,
  terrain = 1u << 3,
  trigger = 1u << 4,
  collectible = 1u << 5,
  interface = 1u << 6,
  all = ~0u
};

enum class bodytype : uint8_t {
  staticbody = 0,
  kinematic
};

struct bodydef final {
  bodytype type{bodytype::staticbody};
  vec2 position{};
  entt::entity entity{entt::null};
  std::optional<vec2> box{};
  bool sensor{false};
};

[[nodiscard]] constexpr b2Vec2 to_b2(const vec2& v) noexcept {
  return {v.x, v.y};
}

[[nodiscard]] constexpr vec2 from_b2(b2Vec2 v) noexcept {
  return {v.x, v.y};
}

[[nodiscard]] constexpr b2AABB aabb(float x, float y, float w, float h) noexcept {
  return {{x, y}, {x + w, y + h}};
}

[[nodiscard]] constexpr b2AABB aabb(const quad& q) noexcept {
  return {{q.x, q.y}, {q.x + q.w, q.y + q.h}};
}

[[nodiscard]] constexpr b2Filter make_filter(category cat, category mask = category::all) noexcept {
  return {static_cast<uint64_t>(cat), static_cast<uint64_t>(mask), 0};
}

[[nodiscard]] constexpr b2QueryFilter make_query_filter(category c = category::all, category mask = category::all) noexcept {
  return {static_cast<uint64_t>(c), static_cast<uint64_t>(mask)};
}

[[nodiscard]] entt::entity entity_from(b2ShapeId shape) noexcept;
[[nodiscard]] bool valid_pair(b2ShapeId a, b2ShapeId b) noexcept;


class world;
class body;

class world final {
public:
  explicit world(const unmarshal::json& node) noexcept;
  ~world() noexcept;

  world(world&& other) noexcept;
  world& operator=(world&& other) noexcept;
  world(const world&) = delete;
  world& operator=(const world&) = delete;

  void step(float delta) noexcept;

  [[nodiscard]] b2WorldId id() const noexcept;
  [[nodiscard]] b2SensorEvents sensor_events() const noexcept;

  template <typename F>
  void raycast(const vec2& origin, float angle, float distance, category mask, F&& callback) const noexcept {
    using hits_t = boost::container::small_vector<std::pair<entt::entity, float>, 16>;
    hits_t hits;

    const auto radians = angle * (std::numbers::pi_v<float> / 180.0f);
    const auto direction = b2Vec2{std::cos(radians) * distance, std::sin(radians) * distance};
    b2World_CastRay(
      _id,
      to_b2(origin),
      direction,
      make_query_filter(category::all, mask),
      [](b2ShapeId shape, b2Vec2, b2Vec2, float fraction, void* userdata) -> float {
        static_cast<hits_t*>(userdata)->emplace_back(entity_from(shape), fraction);
        return 1.0f;
      },
      &hits
    );

    std::ranges::sort(hits, {}, &std::pair<entt::entity, float>::second);
    for (const auto& [entity, fraction] : hits) {
      callback(entity);
    }
  }

  template <typename F>
  void query_aabb(const b2AABB& aabb, category mask, F&& callback) const noexcept {
    struct context { F* fn; };
    context ctx{&callback};
    b2World_OverlapAABB(_id, aabb, make_query_filter(category::all, mask),
      [](b2ShapeId shape, void* userdata) -> bool {
        auto* ctx = static_cast<context*>(userdata);
        return (*ctx->fn)(shape, entity_from(shape));
      }, &ctx);
  }

private:
  b2WorldId _id{};
  float _accumulator{.0f};
};

class body final {
public:
  body() noexcept = default;
  ~body() noexcept;

  body(body&& other) noexcept;
  body& operator=(body&& other) noexcept;
  body(const body&) = delete;
  body& operator=(const body&) = delete;

  [[nodiscard]] static body create(world& w, const bodydef& def) noexcept;

  void attach_sensor_if_changed(float hx, float hy) noexcept;
  void detach() noexcept;
  void set_transform(const vec2& position, float angle) noexcept;

private:
  void attach(float hx, float hy, bool sensor) noexcept;
  void destroy() noexcept;
  [[nodiscard]] bool has_shape() const noexcept;
  [[nodiscard]] b2BodyId id() const noexcept;

  struct alignas(8) cache final {
    float hx{0};
    float hy{0};
  };

  b2BodyId _body{};
  b2ShapeId _shape{};
  cache _cache;
};

}
