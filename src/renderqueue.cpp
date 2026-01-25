#include "renderqueue.hpp"

#include "components.hpp"
#include "particlepool.hpp"
#include "pixmap.hpp"

renderqueue::renderqueue(const entt::registry& registry, const particlepool& particlepool) noexcept
    : _registry(registry), _particlepool(particlepool) {}

void renderqueue::update() noexcept {
  _drawables.clear();

  const auto& batches = _particlepool.batches();
  const auto count = _registry.view<renderable>().size() + batches.size();

  if (_drawables.capacity() < count) [[unlikely]] {
    _drawables.reserve(count);
  }

  for (auto&& [entity, rn] : _registry.view<renderable>().each()) {
    if (rn.visible) [[likely]] {
      _drawables.push_back({static_cast<int16_t>(rn.z), drawablekind::entity, {.entity = entity}});
    }
  }

  for (const auto& [_, batch] : batches) {
    _drawables.push_back({batch->z, drawablekind::batch, {.batch = batch.get()}});
  }

  std::sort(_drawables.begin(), _drawables.end(), [](const drawslice& a, const drawslice& b) noexcept {
    return a.z < b.z;
  });
}

void renderqueue::draw() const noexcept {
  if (_drawables.empty()) [[unlikely]] return;

  for (const auto& item : _drawables) {
    switch (item.kind) {
    case drawablekind::entity: {
      const auto entity = item.entity;

      const auto* pb = _registry.try_get<playback>(entity);
      if (!pb || !pb->timeline || pb->timeline->frames.empty()) [[unlikely]] continue;

      const auto& [tr, tn, sp, fl, dr] = _registry.get<transform, tint, sprite, orientation, ::drawable>(entity);

      const auto& frame = pb->timeline->frames[pb->current_frame];
      const auto& q = frame.quad;

      sp.pixmap->draw(q.x, q.y, q.w, q.h, dr.x, dr.y, dr.w, dr.h, tr.angle, tn.a, fl.flip);
      break;
    }
    case drawablekind::batch: {
      const auto* batch = item.batch;

      SDL_RenderGeometry(
        renderer,
        static_cast<SDL_Texture*>(*batch->pixmap),
        batch->vertices.data(),
        static_cast<int>(batch->vertices.size()),
        batch->indices.data(),
        static_cast<int>(batch->indices.size())
      );
      break;
    }
    }
  }
}
