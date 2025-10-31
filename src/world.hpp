#pragma once

#include <box2d/box2d.h>

namespace framework {
static inline b2AABB to_aabb(float x0, float y0, float x1, float y1) noexcept {
  b2AABB a{};
  a.lowerBound = b2Vec2(x0, y0);
  a.upperBound = b2Vec2(x1, y1);
  return a;
}

template <class OutIt>
static bool collect(b2ShapeId shape, void* context) {
  auto* it = static_cast<OutIt*>(context);
  const auto body = b2Shape_GetBody(shape);
  const auto id = static_cast<uint64_t>(
    reinterpret_cast<uintptr_t>(b2Body_GetUserData(body)));
  *(*it)++ = id;
  return true;
}

class world final {
public:
  world(std::shared_ptr<graphics::renderer> renderer) noexcept;
  ~world() noexcept;

  void add(const std::shared_ptr<object>& object);
  void remove(uint64_t id);

  template <class OutIt>
  void query(float x, float y, OutIt out) {
    constexpr auto epsilon = std::numeric_limits<float>::epsilon();
    const auto aabb = to_aabb(x - epsilon, y - epsilon, x + epsilon, y + epsilon);
    auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect<OutIt>, &out);
  }

  template <class OutIt>
  void query(float x, float y, float w, float h, OutIt out) {
    constexpr auto epsilon = std::numeric_limits<float>::epsilon();
    const auto xw = x + w;
    const auto yh = y + h;
    const auto x0 = (x < xw ? x : xw) - epsilon;
    const auto y0 = (y < yh ? y : yh) - epsilon;
    const auto x1 = (x > xw ? x : xw) + epsilon;
    const auto y1 = (y > yh ? y : yh) + epsilon;

    const auto aabb = to_aabb(x0, y0, x1, y1);
    auto filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &collect<OutIt>, &out);
  }

  void update(float delta) noexcept;
  void draw() const noexcept;

  operator b2WorldId() const noexcept;

protected:
  void notify(uint64_t aid, uint64_t bid) const;

private:
  b2WorldId _world;
  std::shared_ptr<graphics::renderer> _renderer;
  std::vector<uint64_t> _dirties;
  std::unordered_map<uint64_t, b2BodyId> _bodies;
  std::unordered_map<uint64_t, std::weak_ptr<object>> _objects;

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
