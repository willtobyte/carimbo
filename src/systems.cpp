#include "systems.hpp"

#include "components.hpp"
#include "geometry.hpp"

void animationsystem::update(entt::registry& registry, uint64_t now) noexcept {
  auto view = registry.view<animator, state>();

  for (auto entity : view) {
    auto& an = view.get<animator>(entity);
    auto& st = view.get<state>(entity);

    if (!st.action.has_value()) continue;

    auto& tl = an.timelines[st.action.value()];

    if (st.dirty) {
      st.current_frame = 0;
      st.tick = now;
      st.dirty = false;
      continue;
    }

    if (tl.frames.empty()) continue;

    const auto& frame = tl.frames[st.current_frame];
    if (frame.duration == 0 || now - st.tick < static_cast<uint64_t>(frame.duration)) {
      continue;
    }

    st.tick = now;

    if (st.current_frame + 1 >= tl.frames.size()) {
      if (!tl.next.empty()) {
        st.action = tl.next;
        st.current_frame = 0;
        st.dirty = true;
      } else {
        st.current_frame = 0;
      }
    } else {
      st.current_frame++;
    }
  }
}

void physicssystem::update(entt::registry& registry, b2WorldId world, float delta) noexcept {
  auto view = registry.view<transform, animator, state, physics>();

  for (auto entity : view) {
    auto& tr = view.get<transform>(entity);
    auto& an = view.get<animator>(entity);
    auto& st = view.get<state>(entity);
    auto& ph = view.get<physics>(entity);

    if (!ph.enabled) {
      continue;
    }

    if (!ph.is_valid() && ph.dirty) {
      auto bodyDef = b2DefaultBodyDef();
      bodyDef.type = static_cast<b2BodyType>(ph.type);
      bodyDef.position = b2Vec2{tr.position.x, tr.position.y};
      bodyDef.rotation = b2MakeRot(static_cast<float>(tr.angle));

      bodyDef.userData = id_to_userdata(static_cast<uint64_t>(entity));

      ph.body = b2CreateBody(world, &bodyDef);
    }

    if (ph.is_valid()) {
      b2Body_SetTransform(ph.body, b2Vec2{tr.position.x, tr.position.y}, b2MakeRot(static_cast<float>(tr.angle)));
    }

    if (ph.is_valid() && ph.dirty) {
      if (b2Shape_IsValid(ph.shape)) {
        b2DestroyShape(ph.shape, false);
      }

      if (!st.action) continue;

      const auto& opt = an[*st.action].box;
      if (opt) {
        const auto& box = *opt;

        const auto w = box.upperBound.x - box.lowerBound.x;
        const auto h = box.upperBound.y - box.lowerBound.y;
        const auto cx = box.lowerBound.x + w * .5f;
        const auto cy = box.lowerBound.y + h * .5f;

        // const auto sw = w * tr.scale;
        // const auto sh = h * tr.scale;

        // const auto dx = w * (tr.scale - 1.0f) * .5f;
        // const auto dy = h * (tr.scale - 1.0f) * .5f;

        auto polygon = b2MakeOffsetBox(
          w * .5f,
          h * .5f,
          b2Vec2{cx, cy},
          b2Rot_identity);

        auto def = b2DefaultShapeDef();

        ph.shape = b2CreatePolygonShape(ph.body, &def, &polygon);
      }

      ph.dirty = false;
    }
  }
}
