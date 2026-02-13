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
  for (auto&& [entity, at, s, d] : _view.each()) {
    const bool refresh = d.is(dirtable::animation) | !s.timeline;

    if (refresh) [[unlikely]] {
      s.timeline = resolve_timeline(*at, s.action);
      s.current = 0;
      s.tick = now;
      s.finished = false;
      d.clear(dirtable::animation);
      d.mark(dirtable::render | dirtable::physics);
      if (s.timeline) {
        if (const auto* a = _entt.try_get<animatable>(entity)) {
          a->on_begin();
        }
      }
    }

    if (!s.timeline || s.timeline->frames.empty()) [[unlikely]] {
      continue;
    }

    if (s.finished) [[unlikely]] {
      continue;
    }

    const auto& tl = *s.timeline;

    assert(!tl.frames.empty() && "timeline must have frames");
    assert(s.current < tl.frames.size() && "frame index out of bounds");

    const auto& frame = tl.frames[s.current];
    const auto elapsed = now - s.tick;
    const auto ready = frame.duration > 0 && elapsed >= static_cast<uint64_t>(frame.duration);

    if (!ready) [[likely]] {
      continue;
    }

    s.tick = now;

    const auto last = s.current + 1 >= tl.frames.size();
    const auto next = tl.next != empty;
    const auto keep = last && tl.oneshot && !next;
    const auto previous = s.current;

    if (!keep) [[likely]] {
      s.current = last ? 0 : s.current + 1;
    }

    if (previous != s.current) {
      d.mark(dirtable::render);
    }

    if (last & next) {
      s.action = tl.next;
      d.mark(dirtable::animation);
    } else if (last & tl.oneshot) {
      if (const auto* a = _entt.try_get<animatable>(entity)) {
        a->on_end();
      }

      s.finished = true;
    }
  }
}

void physicssystem::update(float delta) {
  _group.each(
    [](entt::entity, const transform& t, physics::body& body, const playback& p, const renderable& rn, const tint& tn, dirtable& d) {
      const auto hitbox = rn.visible && tn.a > 0 && p.timeline && p.timeline->hitbox.has_value();

      if (!hitbox) [[unlikely]] {
        body.detach();
        d.clear(dirtable::physics);
        return;
      }

      if (!d.is(dirtable::physics)) [[likely]] {
        const auto& box = *p.timeline->hitbox;
        const auto px = t.position.x + box.x + box.w * .5f;
        const auto py = t.position.y + box.y + box.h * .5f;
        const auto angle = static_cast<float>(t.angle) * DEGREES_TO_RADIANS;
        body.transform({px, py}, angle);
        return;
      }

      const auto& box = *p.timeline->hitbox;
      const auto hx = box.w * t.scale * .5f;
      const auto hy = box.h * t.scale * .5f;

      const auto px = t.position.x + box.x + box.w * .5f;
      const auto py = t.position.y + box.y + box.h * .5f;
      const auto angle = static_cast<float>(t.angle) * DEGREES_TO_RADIANS;

      body.attach_sensor(hx, hy);
      body.transform({px, py}, angle);
      d.clear(dirtable::physics);
    });

  const auto& interning = _registry.ctx().get<::interning>();

  _world.step(delta, [&](const b2SensorEvents& events) {
    for (auto i = 0; i < events.beginCount; ++i) {
      const auto& event = events.beginEvents[i];
      if (!physics::valid_pair(event.sensorShapeId, event.visitorShapeId)) [[unlikely]] continue;

      const auto sensor = physics::entity_from(event.sensorShapeId);
      const auto visitor = physics::entity_from(event.visitorShapeId);

      const auto* c = _registry.try_get<collidable>(sensor);
      const auto* m = _registry.try_get<metadata>(visitor);

      if (c && m) [[likely]] {
        c->on_collision(static_cast<uint64_t>(visitor), interning.lookup(m->kind));
      }
    }

    for (auto i = 0; i < events.endCount; ++i) {
      const auto& event = events.endEvents[i];
      if (!physics::valid_pair(event.sensorShapeId, event.visitorShapeId)) [[unlikely]] continue;

      const auto sensor = physics::entity_from(event.sensorShapeId);
      const auto visitor = physics::entity_from(event.visitorShapeId);

      const auto* c = _registry.try_get<collidable>(sensor);
      const auto* m = _registry.try_get<metadata>(visitor);

      if (c && m) [[likely]] {
        c->on_collision_end(static_cast<uint64_t>(visitor), interning.lookup(m->kind));
      }
    }
  });

  static constexpr std::string_view directions[] = {"left", "right", "top", "bottom"};

  const auto& camera = *_camera;

  auto sbview = _registry.view<screenboundable, const physics::body>();
  for (auto&& [entity, sb, body] : sbview.each()) {
    if (!body.has_shape()) continue;

    const auto aabb = b2Shape_GetAABB(body.shape);

    uint8_t current = 0;
    if (aabb.upperBound.x < camera.x)             current |= screenboundable::left;
    if (aabb.lowerBound.x > camera.x + camera.w)  current |= screenboundable::right;
    if (aabb.upperBound.y < camera.y)             current |= screenboundable::top;
    if (aabb.lowerBound.y > camera.y + camera.h)  current |= screenboundable::bottom;

    const auto exited  = static_cast<uint8_t>(current & ~sb.previous);
    const auto entered = static_cast<uint8_t>(sb.previous & ~current);

    for (uint8_t bit = 0; bit < 4; ++bit) {
      const auto mask = static_cast<uint8_t>(1u << bit);
      if (exited & mask)  sb.on_screen_exit(directions[bit]);
      if (entered & mask) sb.on_screen_enter(directions[bit]);
    }

    sb.previous = current;
  }
}

void rendersystem::update() noexcept {
  _view.each([](const renderable& rn, const transform& tr, const tint&, const sprite&, const playback& pb, const orientation&, dirtable& d, drawable& dr) {
    if (!rn.visible || !pb.timeline || pb.timeline->frames.empty()) [[unlikely]] {
      return;
    }

    if (!d.is(dirtable::render)) [[likely]] {
      return;
    }

    const auto& tl = *pb.timeline;

    assert(!tl.frames.empty() && "timeline must have frames");
    assert(pb.current < tl.frames.size() && "frame index out of bounds");

    const auto& frame = tl.frames[pb.current];
    const auto& q = frame.quad;

    const auto hw = q.w * .5f;
    const auto hh = q.h * .5f;
    const auto sw = q.w * tr.scale;
    const auto sh = q.h * tr.scale;
    dr.x = frame.offset_x + tr.position.x + hw - sw * .5f;
    dr.y = frame.offset_y + tr.position.y + hh - sh * .5f;
    dr.w = sw;
    dr.h = sh;
    d.clear(dirtable::render);
  });
}

void scriptsystem::update(float delta) {
  _view.each([delta](scriptable& sc) {
    sc.on_loop(delta);
  });
}

void velocitysystem::update(float delta) {
  _view.each([delta](transform& t, const velocity& v, dirtable& d) {
    if (v.value.x == 0.f && v.value.y == 0.f) [[likely]] {
      return;
    }

    t.position.x += v.value.x * delta;
    t.position.y += v.value.y * delta;
    d.mark(dirtable::render);
  });
}
