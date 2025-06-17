#include "tilemap.hpp"

using namespace framework;

tilemap::tilemap(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<resourcemanager> resourcemanager, const std::string &name)
  : _resourcemanager(std::move(resourcemanager)) {
  UNUSED(name);

  int32_t lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(*renderer, &lw, &lh, &mode);

  float_t sx, sy;
  SDL_GetRenderScale(*renderer, &sx, &sy);

  const auto width = static_cast<float_t>(lw) / sx;
  const auto height = static_cast<float_t>(lh) / sy;
  _view = {.0f, .0f, width, height};
}

void tilemap::update(float_t delta) {
  UNUSED(delta);
  if (!_target || !_tileset) {
    return;
  }

  const constexpr auto smooth = 3.0f;

  const auto position = _target->position();
  const auto vw  = _view.width();
  const auto vh  = _view.height();

  const auto dx = position.x() - vw * 0.5f;
  const auto dy = position.y() - vh * 0.5f;
  const auto damping = 1.0f - std::exp(-smooth * delta);

  _view.set_position({
    (dx - _view.x()) * damping,
    (dy - _view.y()) * damping
  });
}

void tilemap::draw() const {
  if (!_tileset) {
    return;
  }

  // TODO _tileset->draw(const geometry::rectangle &source, const geometry::rectangle &destination)
}

void tilemap::set_target(std::shared_ptr<object> object) {
  _target = std::move(object);
}
