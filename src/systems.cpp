#include "systems.hpp"

#include "components.hpp"
#include "constant.hpp"
#include "geometry.hpp"

void animationsystem::update(entt::registry& registry, uint64_t now) noexcept {
  auto view = registry.view<atlas, playback>();

  view.each([now](const atlas& at, playback& s) {
    if (!s.action.has_value()) [[unlikely]] {
      s.cache = nullptr;
      return;
    }

    if (s.dirty || !s.cache) [[unlikely]] {
      const auto it = at.timelines.find(*s.action);
      if (it == at.timelines.end()) [[unlikely]] {
        s.cache = nullptr;
        return;
      }

      s.cache = &it->second;
      s.current_frame = 0;
      s.tick = now;
      s.dirty = false;
      return;
    }

    const auto& timeline = *s.cache;

    if (timeline.frames.empty()) [[unlikely]] {
      return;
    }

    if (s.current_frame >= timeline.frames.size()) [[unlikely]] {
      s.current_frame = 0;
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
      if (next) {
        s.action = timeline.next;
        s.current_frame = 0;
        s.dirty = true;
      } else if (timeline.oneshot) {
        s.action = std::nullopt;
      } else {
        s.current_frame = 0;
      }
    } else [[likely]] {
      ++s.current_frame;
    }
  });
}

void physicssystem::update(entt::registry& registry, b2WorldId world, float delta) noexcept {
  auto view = registry.view<transform, playback, physics, renderable>();

  view.each([world](entt::entity entity, const transform& t, const playback& s, physics& p, const renderable& rn) {
    if (!p.enabled) [[unlikely]] {
      return;
    }

    const bool valid = p.is_valid();

    if (!rn.visible) [[unlikely]] {
      if (!valid) {
        return;
      }

      if (b2Shape_IsValid(p.shape)) {
        b2DestroyShape(p.shape, false);
        p.shape = b2ShapeId{};
      }
      b2DestroyBody(p.body);
      p.body = b2BodyId{};
      p.dirty = true;
      return;
    }

    if (!s.cache) [[unlikely]] {
      return;
    }

    const auto& timeline = *s.cache;
    const auto& opt = timeline.hitbox;

    if (!opt) [[unlikely]] {
      return;
    }

    const auto& hitbox = *opt;

    const auto bw = hitbox.upperBound.x - hitbox.lowerBound.x;
    const auto bh = hitbox.upperBound.y - hitbox.lowerBound.y;

    const auto hx = bw * t.scale * 0.5f;
    const auto hy = bh * t.scale * 0.5f;

    const auto cx = bw * 0.5f;
    const auto cy = bh * 0.5f;
    const auto px = t.position.x + hitbox.lowerBound.x + cx;
    const auto py = t.position.y + hitbox.lowerBound.y + cy;

    const auto position = b2Vec2{px, py};
    const auto radians = static_cast<float>(t.angle) * DEGREES_TO_RADIANS;
    const auto rotation = b2MakeRot(radians);

    if (!valid) [[unlikely]] {
      auto bdef = b2DefaultBodyDef();
      bdef.type = static_cast<b2BodyType>(p.type);
      bdef.position = position;
      bdef.rotation = rotation;
      bdef.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));

      p.body = b2CreateBody(world, &bdef);

      const auto poly = b2MakeBox(hx, hy);
      auto sdef = b2DefaultShapeDef();
      p.shape = b2CreatePolygonShape(p.body, &sdef, &poly);
      p.dirty = false;
      return;
    }

    b2Body_SetTransform(p.body, position, rotation);

    if (!p.dirty) [[likely]] {
      return;
    }

    if (b2Shape_IsValid(p.shape)) {
      b2DestroyShape(p.shape, false);
    }

    const auto poly = b2MakeBox(hx, hy);
    auto sdef = b2DefaultShapeDef();
    p.shape = b2CreatePolygonShape(p.body, &sdef, &poly);
    p.dirty = false;
  });
}

void rendersystem::draw(const entt::registry& registry) const noexcept {
  auto view = registry.view<renderable, transform, tint, sprite, playback, orientation>();

  view.each([](const renderable& rn, const transform& tr, const tint& tn, const sprite& sp, const playback& st, const orientation& fl) {
    if (!rn.visible) [[unlikely]] {
      return;
    }

    if (!st.cache) [[unlikely]] {
      return;
    }

    const auto& timeline = *st.cache;

    if (timeline.frames.empty()) [[unlikely]] {
      return;
    }

    if (st.current_frame >= timeline.frames.size()) [[unlikely]] {
      return;
    }

    const auto& frame = timeline.frames[st.current_frame];

    const auto hw = frame.quad.w * 0.5f;
    const auto hh = frame.quad.h * 0.5f;

    const auto sw = frame.quad.w * tr.scale;
    const auto sh = frame.quad.h * tr.scale;

    const auto cx = frame.offset.x + tr.position.x + hw;
    const auto cy = frame.offset.y + tr.position.y + hh;

    const auto fx = cx - sw * 0.5f;
    const auto fy = cy - sh * 0.5f;

    sp.pixmap->draw(
      frame.quad.x, frame.quad.y,
      frame.quad.w, frame.quad.h,
      fx, fy,
      sw, sh,
      tr.angle,
      tn.a,
      fl.flip
    );
  });
}