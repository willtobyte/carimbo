#include "systems.hpp"

#include "components.hpp"
#include "constant.hpp"
#include "geometry.hpp"

namespace {
[[nodiscard]] inline const timeline* resolve_timeline(const atlas& at, symbol action) noexcept {
  if (action == no_action) [[unlikely]] {
    return nullptr;
  }

  return at.find(action);
}

static void destroy_body(physics& p) noexcept {
  if (b2Shape_IsValid(p.shape)) {
    b2DestroyShape(p.shape, false);
    p.shape = b2ShapeId{};
  }

  b2DestroyBody(p.body);
  p.body = b2BodyId{};
  p.dirty = true;
}

static void patch_shape(physics& p, float hx, float hy) noexcept {
  if (b2Shape_IsValid(p.shape)) {
    b2DestroyShape(p.shape, false);
  }

  const auto poly = b2MakeBox(hx, hy);
  auto def = b2DefaultShapeDef();
  def.isSensor = true;
  def.enableSensorEvents = true;
  p.shape = b2CreatePolygonShape(p.body, &def, &poly);
  p.dirty = false;
}

struct result final {
  b2Vec2 position;
  b2Rot rotation;
  float hx;
  float hy;
};

[[nodiscard]] static result compute_hitbox(const b2AABB& hitbox, const transform& t) noexcept {
  const auto bw = hitbox.upperBound.x - hitbox.lowerBound.x;
  const auto bh = hitbox.upperBound.y - hitbox.lowerBound.y;

  return {
    .position = {
      t.position.x + hitbox.lowerBound.x + bw * 0.5f,
      t.position.y + hitbox.lowerBound.y + bh * 0.5f
    },
    .rotation = b2MakeRot(static_cast<float>(t.angle) * DEGREES_TO_RADIANS),
    .hx = bw * t.scale * 0.5f,
    .hy = bh * t.scale * 0.5f
  };
}
}

void animationsystem::update(uint64_t now) noexcept {
  _group.each([now](const std::shared_ptr<const atlas>& at, playback& s, callbacks& c) {
    const bool refresh = s.dirty | !s.timeline;

    if (refresh) [[unlikely]] {
      s.timeline = resolve_timeline(*at, s.action);
      s.current_frame = 0;
      s.tick = now;
      s.dirty = false;
      if (s.timeline) {
        c.on_begin();
      }
    }

    if (!s.timeline || s.timeline->frames.empty()) [[unlikely]] {
      return;
    }

    const auto& tl = *s.timeline;

    assert(!tl.frames.empty() && "timeline must have frames");
    assert(s.current_frame < tl.frames.size() && "frame index out of bounds");

    const auto& frame = tl.frames[s.current_frame];
    const auto elapsed = now - s.tick;
    const auto ready = frame.duration > 0 && elapsed >= static_cast<uint64_t>(frame.duration);

    if (!ready) [[likely]] {
      return;
    }

    s.tick = now;

    const auto is_last = s.current_frame + 1 >= tl.frames.size();
    const auto has_next = tl.next != no_action;

    s.current_frame = is_last ? 0 : s.current_frame + 1;

    if (is_last & has_next) {
      s.action = tl.next;
      s.dirty = true;
    } else if (is_last & tl.oneshot) {
      s.action = no_action;
      s.timeline = nullptr;
      c.on_end();
    }
  });
}

void physicssystem::update(b2WorldId world, [[maybe_unused]] float delta) noexcept {
  b2World_Step(world, FIXED_TIMESTEP, WORLD_SUBSTEPS);

  const auto events = b2World_GetSensorEvents(world);

  for (int i = 0; i < events.beginCount; ++i) {
    const auto& event = events.beginEvents[i];
    if (!b2Shape_IsValid(event.sensorShapeId) || !b2Shape_IsValid(event.visitorShapeId)) [[unlikely]] {
      continue;
    }
    const auto sd = b2Body_GetUserData(b2Shape_GetBody(event.sensorShapeId));
    const auto vd = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
    if (!sd || !vd) [[unlikely]] continue;

    const auto sensor = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(sd));
    const auto visitor = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(vd));

    const auto* sc = _registry.try_get<callbacks>(sensor);
    const auto* vm = _registry.try_get<metadata>(visitor);

    if (sc && vm) [[likely]] {
      sc->on_collision(static_cast<uint64_t>(visitor), vm->kind);
    }
  }

  for (int i = 0; i < events.endCount; ++i) {
    const auto& event = events.endEvents[i];
    if (!b2Shape_IsValid(event.sensorShapeId) || !b2Shape_IsValid(event.visitorShapeId)) [[unlikely]] {
      continue;
    }
    const auto sd = b2Body_GetUserData(b2Shape_GetBody(event.sensorShapeId));
    const auto vd = b2Body_GetUserData(b2Shape_GetBody(event.visitorShapeId));
    if (!sd || !vd) [[unlikely]] continue;

    const auto sensor = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(sd));
    const auto visitor = static_cast<entt::entity>(reinterpret_cast<std::uintptr_t>(vd));

    const auto* sc = _registry.try_get<callbacks>(sensor);
    const auto* vm = _registry.try_get<metadata>(visitor);

    if (sc && vm) [[likely]] {
      sc->on_collision_end(static_cast<uint64_t>(visitor), vm->kind);
    }
  }

  _group.each(
    [world](entt::entity entity, const transform& t, physics& p, const playback& s, const renderable& rn) {
      if (!p.enabled) [[unlikely]] {
        return;
      }

      const auto valid = p.is_valid();
      const auto destroy = !rn.visible & valid;
      if (destroy) [[unlikely]] {
        destroy_body(p);
        return;
      }

      const auto collidable = rn.visible & (s.timeline != nullptr) && s.timeline->hitbox.has_value();

      if (!collidable) [[unlikely]] {
        return;
      }

      const auto params = compute_hitbox(*s.timeline->hitbox, t);

      if (!valid) [[unlikely]] {
        auto def = b2DefaultBodyDef();
        def.type = static_cast<b2BodyType>(p.type);
        def.position = params.position;
        def.rotation = params.rotation;
        def.userData = reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity));
        p.body = b2CreateBody(world, &def);

        patch_shape(p, params.hx, params.hy);

        return;
      }

      b2Body_SetTransform(p.body, params.position, params.rotation);

      if (p.dirty) [[unlikely]] {
        patch_shape(p, params.hx, params.hy);
      }
    });
}

void rendersystem::draw() const noexcept {
  auto view = _registry.view<renderable, transform, tint, sprite, playback, orientation>();
  view.use<renderable>();
  view.each([](const renderable& rn, const transform& tr, const tint& tn, const sprite& sp, const playback& pb, const orientation& fl) {
      if (!rn.visible || !pb.timeline || pb.timeline->frames.empty()) [[unlikely]] {
        return;
      }

      const auto& tl = *pb.timeline;

      assert(!tl.frames.empty() && "timeline must have frames");
      assert(pb.current_frame < tl.frames.size() && "frame index out of bounds");

      const auto& frame = tl.frames[pb.current_frame];
      const auto& q = frame.quad;

      const auto hw = q.w * 0.5f;
      const auto hh = q.h * 0.5f;
      const auto sw = q.w * tr.scale;
      const auto sh = q.h * tr.scale;
      const auto fx = frame.offset.x + tr.position.x + hw - sw * 0.5f;
      const auto fy = frame.offset.y + tr.position.y + hh - sh * 0.5f;

      sp.pixmap->draw(q.x, q.y, q.w, q.h, fx, fy, sw, sh, tr.angle, tn.a, fl.flip);
    });
}

void scriptsystem::update(float delta) {
  _view.each([delta](scriptable& sc) {
    sc.on_loop(delta);
  });
}
