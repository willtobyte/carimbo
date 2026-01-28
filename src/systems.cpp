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

    const auto is_last = s.current + 1 >= tl.frames.size();
    const auto has_next = tl.next != empty;
    const auto keep_last = is_last && tl.oneshot && !has_next;

    const auto prev_frame = s.current;
    if (!keep_last) [[likely]] {
      s.current = is_last ? 0 : s.current + 1;
    }

    if (prev_frame != s.current) {
      d.mark(dirtable::render);
    }

    if (is_last & has_next) {
      s.action = tl.next;
      d.mark(dirtable::animation);
    } else if (is_last & tl.oneshot) {
      if (const auto* a = _entt.try_get<animatable>(entity)) {
        a->on_end();
      }

      s.finished = true;
    }
  }
}

void physicssystem::update(float delta) {
  _world.step(delta);

  const auto events = _world.sensor_events();
  const auto& interning = _registry.ctx().get<::interning>();

  for (int i = 0; i < events.beginCount; ++i) {
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

  for (int i = 0; i < events.endCount; ++i) {
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

void rendersystem::draw() const noexcept {
  _view.each([](const renderable& rn, const transform& tr, const tint& tn, const sprite& sp, const playback& pb, const orientation& fl, const dirtable&, const drawable& dr) {
    if (!rn.visible || !pb.timeline || pb.timeline->frames.empty()) [[unlikely]] {
      return;
    }

    const auto& frame = pb.timeline->frames[pb.current];
    const auto& q = frame.quad;

    sp.pixmap->draw(q.x, q.y, q.w, q.h, dr.x, dr.y, dr.w, dr.h, tr.angle, tn.a, fl.flip);
  });
}

void scriptsystem::update(float delta) {
  _view.each([delta](scriptable& sc) {
    sc.on_loop(delta);
  });
}

void velocitysystem::update(float delta) {
  _view.each([delta](transform& t, const velocity& v) {
    t.position.x += v.value.x * delta;
    t.position.y += v.value.y * delta;
  });
}
