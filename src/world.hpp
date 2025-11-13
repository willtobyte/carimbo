#pragma once

#include "common.hpp"

namespace framework {
class objectmanager;

[[nodiscard]] constexpr b2AABB to_aabb(const float x0, const float y0, const float x1, const float y1) noexcept {
  auto a = b2AABB{};
  a.lowerBound = b2Vec2(x0, y0);
  a.upperBound = b2Vec2(x1, y1);
  return a;
}

template <class OutIt>
[[nodiscard]] static bool collect(const b2ShapeId shape, void* const context) {
  auto* const it = static_cast<OutIt*>(context);
  const auto body = b2Shape_GetBody(shape);
  const auto data = b2Body_GetUserData(body);
  if (!data) return true;

  const auto id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(data));
  *(*it)++ = id;
  return true;
}

class world final {
public:
  explicit world(std::shared_ptr<graphics::renderer> renderer);
  ~world();

  template <class OutIt>
  void query(const float x, const float y, OutIt out) const {
    const auto aabb = to_aabb(x - epsilon, y - epsilon, x + epsilon, y + epsilon);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect<OutIt>, &out);
  }

  template <class OutIt>
  void query(const float x, const float y, const float w, const float h, OutIt out) const {
    const auto xw = x + w;
    const auto yh = y + h;
    const auto x0 = (x < xw ? x : xw) - epsilon;
    const auto y0 = (y < yh ? y : yh) - epsilon;
    const auto x1 = (x > xw ? x : xw) + epsilon;
    const auto y1 = (y > yh ? y : yh) + epsilon;

    const auto aabb = to_aabb(x0, y0, x1, y1);
    const auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect<OutIt>, &out);
  }

  void update(float delta);
  void draw() const;

  void set_objectmanager(std::weak_ptr<objectmanager> objectmanager);

  [[nodiscard]] operator b2WorldId() const noexcept;

protected:
  void notify(uint64_t id_a, uint64_t id_b) const;

private:
  b2WorldId _world;
  float _accumulator{.0f};
  std::shared_ptr<graphics::renderer> _renderer;
  std::weak_ptr<objectmanager> _objectmanager;
  std::unordered_set<std::pair<uint64_t, uint64_t>, boost::hash<std::pair<uint64_t, uint64_t>>> _collisions;

  std::shared_ptr<envelopepool_impl> _envelopepool = envelopepool::instance();
};
}
