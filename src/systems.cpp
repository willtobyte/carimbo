#include "systems.hpp"

#include "components.hpp"
#include "constant.hpp"
#include "geometry.hpp"
#include "physics.hpp"

namespace {
[[nodiscard]] inline const timeline* resolve_timeline(const atlas& at, symbol action) noexcept {
  if (action == empty) [[unlikely]] {
    return nullptr;
  }

  return at.find(action);
}
}

void animationsystem::update(uint64_t now) {
  for (auto&& [entity, at, s] : _view.each()) {
    const bool refresh = s.dirty | !s.timeline;

    if (refresh) [[unlikely]] {
      s.timeline = resolve_timeline(*at, s.action);
      s.current_frame = 0;
      s.tick = now;
      s.dirty = false;
      if (s.timeline) {
        if (const auto* a = _entt.try_get<animatable>(entity)) {
          a->on_begin();
        }
      }
    }

    if (!s.timeline || s.timeline->frames.empty()) [[unlikely]] {
      continue;
    }

    const auto& tl = *s.timeline;

    assert(!tl.frames.empty() && "timeline must have frames");
    assert(s.current_frame < tl.frames.size() && "frame index out of bounds");

    const auto& frame = tl.frames[s.current_frame];
    const auto elapsed = now - s.tick;
    const auto ready = frame.duration > 0 && elapsed >= static_cast<uint64_t>(frame.duration);

    if (!ready) [[likely]] {
      continue;
    }

    s.tick = now;

    const auto is_last = s.current_frame + 1 >= tl.frames.size();
    const auto has_next = tl.next != empty;

    s.current_frame = is_last ? 0 : s.current_frame + 1;

    if (is_last & has_next) {
      s.action = tl.next;
      s.dirty = true;
    } else if (is_last & tl.oneshot) {
      s.action = empty;
      s.timeline = nullptr;
      if (const auto* a = _entt.try_get<animatable>(entity)) {
        a->on_end();
      }
    }
  }
}

void physicssystem::update(b2WorldId world, float delta) {
  _accumulator += delta;
  while (_accumulator >= FIXED_TIMESTEP) {
    b2World_Step(world, FIXED_TIMESTEP, WORLD_SUBSTEPS);
    _accumulator -= FIXED_TIMESTEP;
  }

  const auto events = b2World_GetSensorEvents(world);

  for (int i = 0; i < events.beginCount; ++i) {
    const auto& event = events.beginEvents[i];
    if (!physics::valid_pair(event.sensorShapeId, event.visitorShapeId)) [[unlikely]] continue;

    const auto sensor = physics::entity_from(event.sensorShapeId);
    const auto visitor = physics::entity_from(event.visitorShapeId);

    const auto* c = _registry.try_get<collidable>(sensor);
    const auto* m = _registry.try_get<metadata>(visitor);

    if (c && m) [[likely]] {
      c->on_collision(static_cast<uint64_t>(visitor), m->kind);
    }
  }

  for (int i = 0; i < events.endCount; ++i) {
    const auto& event = events.endEvents[i];
    if (!physics::valid_pair(event.sensorShapeId, event.visitorShapeId)) [[unlikely]] continue;

    const auto sensor = physics::entity_from(event.sensorShapeId);
    const auto visitor = physics::entity_from(event.visitorShapeId);

    const auto* c = _registry.try_get<collidable>(sensor);
    const auto* m = _registry.try_get<metadata>(visitor);

    if (c && m) [[likely]] {
      c->on_collision_end(static_cast<uint64_t>(visitor), m->kind);
    }
  }

  _group.each(
    [world](entt::entity entity, const transform& t, rigidbody& r, const playback& p, const renderable& rn) {
      if (!r.enabled) [[unlikely]] return;

      const auto valid = r.is_valid();

      if (!rn.visible & valid) [[unlikely]] {
        physics::destroy(r.shape, r.body);
        return;
      }

      const auto collidable = rn.visible & (p.timeline != nullptr) && p.timeline->hitbox.has_value();
      if (!collidable) [[unlikely]] {
        if (valid) physics::destroy(r.shape, r.body);
        return;
      }

      const auto& box = *p.timeline->hitbox;
      const auto bw = box.upperBound.x - box.lowerBound.x;
      const auto bh = box.upperBound.y - box.lowerBound.y;

      const rigidbody::state state{
        {t.position.x + box.lowerBound.x + bw * 0.5f, t.position.y + box.lowerBound.y + bh * 0.5f},
        b2MakeRot(static_cast<float>(t.angle) * DEGREES_TO_RADIANS),
        bw * t.scale * 0.5f,
        bh * t.scale * 0.5f
      };

      if (!valid) [[unlikely]] {
        r.body = physics::make_body(world, static_cast<b2BodyType>(r.type), state.position, state.rotation,
          reinterpret_cast<void*>(static_cast<std::uintptr_t>(entity)));
        r.shape = physics::make_sensor(r.body, state.hx, state.hy);
        r.cache = state;
        return;
      }

      if (r.cache.update_transform(state))
        b2Body_SetTransform(r.body, state.position, state.rotation);

      if (r.cache.update_shape(state)) {
        physics::destroy_shape(r.shape);
        r.shape = physics::make_sensor(r.body, state.hx, state.hy);
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
      const auto fx = frame.offset_x + tr.position.x + hw - sw * 0.5f;
      const auto fy = frame.offset_y + tr.position.y + hh - sh * 0.5f;

      sp.pixmap->draw(q.x, q.y, q.w, q.h, fx, fy, sw, sh, tr.angle, tn.a, fl.flip);
    });
}

void scriptsystem::update(float delta) {
  _view.each([delta](scriptable& sc) {
    sc.on_loop(delta);
  });
}
