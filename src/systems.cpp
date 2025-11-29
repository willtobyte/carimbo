#include "systems.hpp"

#include "components.hpp"
#include "constant.hpp"
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
  auto view = registry.view<transform, animator, state, physics, renderable>();

  view.each([world](const auto entity, const transform& t, const animator& an, const state& s, physics& p, const renderable& rn) {
    if (!p.enabled) [[unlikely]] {
      return;
    }

    if (!rn.visible) [[unlikely]] {
      if (p.is_valid()) {
        if (b2Shape_IsValid(p.shape)) {
          b2DestroyShape(p.shape, false);
          p.shape = b2ShapeId{};
        }
        b2DestroyBody(p.body);
        p.body = b2BodyId{};
        p.dirty = true;
      }
      return;
    }

    if (!s.action) [[unlikely]] {
      return;
    }

    const auto it = an.timelines.find(*s.action);
    if (it == an.timelines.end()) [[unlikely]] {
      return;
    }

    const auto& timeline = it->second;
    const auto& opt = timeline.box;
    if (!opt) [[unlikely]] {
      return;
    }

    const auto& box = *opt;

    const auto bw = box.upperBound.x - box.lowerBound.x;
    const auto bh = box.upperBound.y - box.lowerBound.y;

    const auto hx = bw * t.scale * 0.5f;
    const auto hy = bh * t.scale * 0.5f;

    const auto cx = bw * 0.5f;
    const auto cy = bh * 0.5f;
    const auto px = t.position.x + box.lowerBound.x + cx;
    const auto py = t.position.y + box.lowerBound.y + cy;

    const auto position = b2Vec2{px, py};
    const auto radians = static_cast<float>(t.angle) * DEGREES_TO_RADIANS;
    const auto rotation = b2MakeRot(radians);

    if (!p.is_valid()) [[unlikely]] {
      auto bdef = b2DefaultBodyDef();
      bdef.type = static_cast<b2BodyType>(p.type);
      bdef.position = position;
      bdef.rotation = rotation;
      bdef.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));

      p.body = b2CreateBody(world, &bdef);
    }

    if (p.is_valid()) [[likely]] {
      b2Body_SetTransform(p.body, position, rotation);
    }

    if (p.is_valid() && p.dirty) [[unlikely]] {
      if (b2Shape_IsValid(p.shape)) {
        b2DestroyShape(p.shape, false);
      }

      const auto poly = b2MakeBox(hx, hy);

      auto sdef = b2DefaultShapeDef();
      p.shape = b2CreatePolygonShape(p.body, &sdef, &poly);
      p.dirty = false;
    }
  });
}

void rendersystem::draw(const entt::registry& registry) const noexcept {
  auto view = registry.view<renderable, transform, tint, sprite, animator, state>();

  for (auto entity : view) {
    const auto& [rn, tr, tn, sp, an, st] = view.get<renderable, transform, tint, sprite, animator, state>(entity);
    if (!rn.visible) [[unlikely]] continue;
    if (!st.action.has_value()) [[unlikely]] continue;

    const auto& timeline = an[st.action.value()];
    if (timeline.frames.empty()) [[unlikely]] continue;

    const auto& frame = timeline.frames[st.current_frame];

    const auto sw = frame.quad.w * tr.scale;
    const auto sh = frame.quad.h * tr.scale;

    const auto cx = frame.offset.x + tr.position.x + frame.quad.w * 0.5f;
    const auto cy = frame.offset.y + tr.position.y + frame.quad.h * 0.5f;

    const auto fx = cx - sw * 0.5f;
    const auto fy = cy - sh * 0.5f;

    sp.pixmap->draw(
      frame.quad.x, frame.quad.y,
      frame.quad.w, frame.quad.h,
      fx, fy,
      sw, sh,
      tr.angle,
      tn.a
    );
  }
}
