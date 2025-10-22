#pragma once

#include <box2d/box2d.h>

namespace framework {

class world final {
public:
  world(std::shared_ptr<graphics::renderer> renderer) noexcept;
  ~world() noexcept;

  void add(const std::shared_ptr<object>& object);
  void remove(uint64_t id);

  template <class OutIt>
  void query(float x, float y, OutIt out) {
    struct Helper {
      static bool cb(b2ShapeId shapeId, void* ctx) {
        auto* it = static_cast<OutIt*>(ctx);
        const b2BodyId bodyId = b2Shape_GetBody(shapeId);
        const uint64_t id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyId)));
        *(*it)++ = id;
        return true;
      }
    };

    b2AABB aabb{};
    aabb.lowerBound = b2Vec2(x, y);
    aabb.upperBound = b2Vec2(x, y);

    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &Helper::cb, &out);
  }

  template <class OutIt>
  void query(float x, float y, float w, float h, OutIt out) {
    const float x0 = std::min(x, x + w);
    const float y0 = std::min(y, y + h);
    const float x1 = std::max(x, x + w);
    const float y1 = std::max(y, y + h);

    struct Helper {
      static bool cb(b2ShapeId shapeId, void* ctx) {
        auto* it = static_cast<OutIt*>(ctx);
        const b2BodyId bodyId = b2Shape_GetBody(shapeId);
        const uint64_t id = static_cast<uint64_t>(reinterpret_cast<uintptr_t>(b2Body_GetUserData(bodyId)));
        *(*it)++ = id;
        return true;
      }
    };

    b2AABB aabb{};
    aabb.lowerBound = b2Vec2(x0, y0);
    aabb.upperBound = b2Vec2(x1, y1);

    b2QueryFilter filter = b2DefaultQueryFilter();
    b2World_OverlapAABB(_world, aabb, filter, &Helper::cb, &out);
  }

  void update(float delta) noexcept;
  void draw() const noexcept;

protected:
  void notify(uint64_t aid, uint64_t bid) const;

private:
  std::shared_ptr<graphics::renderer> _renderer;
  std::vector<uint64_t> _dirties;

  b2WorldId _world;
  std::unordered_map<uint64_t, b2BodyId> _bodies;

  std::unordered_map<uint64_t, std::weak_ptr<object>> _objects;

  std::shared_ptr<uniquepool<envelope, framework::envelope_pool_name>> _envelopepool = envelopepool::instance();
};
}
