#include "systems.hpp"

#include "components.hpp"
#include "geometry.hpp"

void animationsystem::update(entt::registry& registry, uint64_t now) noexcept {
  auto view = registry.view<animator, state>();

  view.each([now](const auto entity, const animator& an, state& s) {
    if (!s.action.has_value()) [[unlikely]] {
      return;
    }

    const auto& it = an.timelines.find(*s.action);
    if (it == an.timelines.end()) [[unlikely]] {
      return;
    }

    const auto& timeline = it->second;

    if (s.dirty) [[unlikely]] {
      s.current_frame = 0;
      s.tick = now;
      s.dirty = false;
      return;
    }

    if (timeline.frames.empty()) [[unlikely]] {
      return;
    }

    const auto& frame = timeline.frames[s.current_frame];
    const auto elapsed = now - s.tick;

    if (frame.duration == 0 || elapsed < static_cast<uint64_t>(frame.duration)) [[likely]] {
      return;
    }

    s.tick = now;

    const auto last = s.current_frame + 1 >= timeline.frames.size();
    const auto next = !timeline.next.empty();

    if (last) [[unlikely]] {
      s.current_frame = 0;
      if (next) {
        s.action = timeline.next;
        s.dirty = true;
      }
    } else [[likely]] {
      ++s.current_frame;
    }
  });
}

void physicssystem::update(entt::registry& registry, b2WorldId world, float delta) noexcept {
  auto view = registry.view<transform, animator, state, physics>();

  view.each([world](const auto entity, const transform& t, const animator& an, const state& s, physics& p) {
    if (!p.enabled) [[unlikely]] {
      return;
    }

    if (!p.is_valid() && p.dirty) [[unlikely]] {
      auto bdef = b2DefaultBodyDef();
      bdef.type = static_cast<b2BodyType>(p.type);
      bdef.position = b2Vec2{t.position.x, t.position.y};
      bdef.rotation = b2MakeRot(static_cast<float>(t.angle));
      bdef.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));

      p.body = b2CreateBody(world, &bdef);
    }

    if (p.is_valid()) [[likely]] {
      b2Body_SetTransform(p.body,
                         b2Vec2{t.position.x, t.position.y},
                         b2MakeRot(static_cast<float>(t.angle)));
    }

    if (p.is_valid() && p.dirty) [[unlikely]] {
      if (b2Shape_IsValid(p.shape)) {
        b2DestroyShape(p.shape, false);
      }

      if (!s.action) [[unlikely]] {
        return;
      }

      const auto it = an.timelines.find(*s.action);
      if (it == an.timelines.end()) [[unlikely]] {
        return;
      }

      const auto& opt = it->second.box;
      if (!opt) [[unlikely]] {
        return;
      }

      const auto& box = *opt;

      const auto w = box.upperBound.x - box.lowerBound.x;
      const auto h = box.upperBound.y - box.lowerBound.y;
      const auto hw = w * 0.5f;
      const auto hh = h * 0.5f;
      const auto cx = box.lowerBound.x + hw;
      const auto cy = box.lowerBound.y + hh;

      auto poly = b2MakeOffsetBox(hw, hh, b2Vec2{cx, cy}, b2Rot_identity);

      auto sdef = b2DefaultShapeDef();
      p.shape = b2CreatePolygonShape(p.body, &sdef, &poly);

      p.dirty = false;
    }
  });
}
