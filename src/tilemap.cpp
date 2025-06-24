#include "tilemap.hpp"

using namespace framework;

tilemap::tilemap(
  std::shared_ptr<graphics::renderer> renderer,
  std::shared_ptr<resourcemanager> resourcemanager,
  const std::string& name
) {
  UNUSED(resourcemanager);
  UNUSED(name);

  int32_t lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(*renderer, &lw, &lh, &mode);

  float_t sx, sy;
  SDL_GetRenderScale(*renderer, &sx, &sy);

  const auto width = static_cast<float_t>(lw) / sx;
  const auto height = static_cast<float_t>(lh) / sy;

  _view = { .0f, .0f, width, height };
  _tilesize = 16.f;

  _tileset = resourcemanager->pixmappool()->get("blobs/tilesets/0.png");

  _layers = {
    {1, 0, 0, 1, 0, 1, 0, 1},
    {1, 0, 1, 0, 1, 0, 1, 0},
    {1, 0, 1, 0, 1, 0, 1, 0},
  };

  if (!_tileset) {
    return;
  }

  const auto tiles_per_row = static_cast<uint32_t>(_tileset->width() / _tilesize);

  static constexpr const auto max_index = 255u;

  _tile_sources.resize(max_index + 1);
  for (auto i = 0u; i <= max_index; ++i) {
    const float src_x = static_cast<float_t>(i % tiles_per_row) * _tilesize;
    const float src_y = (static_cast<float_t>(i) / static_cast<float_t>(tiles_per_row)) * _tilesize;

    _tile_sources[i] = geometry::rectangle{ {src_x, src_y}, {_tilesize, _tilesize} };
  }
}

void tilemap::update(float_t delta) noexcept {
  UNUSED(delta);

  if (!_target || !_tileset) {
    return;
  }

  constexpr auto smooth = 3.0f;
  const auto position = _target->position();
  const auto vw = _view.width();
  const auto vh = _view.height();
  const auto dx = position.x() - vw * 0.5f;
  const auto dy = position.y() - vh * 0.5f;
  const auto damping = 1.0f - std::exp(-smooth * delta);

  _view.set_position({
    _view.x() + (dx - _view.x()) * damping,
    _view.y() + (dy - _view.y()) * damping
  });
}

void tilemap::draw() const noexcept {
  if (!_tileset) [[unlikely]] {
    return;
  }

  const auto view_x0 = _view.x();
  const auto view_y0 = _view.y();
  const auto view_x1 = view_x0 + _view.width();
  const auto view_y1 = view_y0 + _view.height();

  const size_t map_height_tiles = _layers.size();

  for (auto y = 0u; y < map_height_tiles; ++y) {
    const size_t map_width_tiles = _layers[y].size();
    const float dest_y = static_cast<float_t>(y) * _tilesize;

    if (dest_y + _tilesize < view_y0 || dest_y > view_y1) [[likely]] {
      continue;
    }

    for (auto x = 0u; x < map_width_tiles; ++x) {
      const float dest_x = static_cast<float_t>(x) * _tilesize;

      if (dest_x + _tilesize < view_x0 || dest_x > view_x1) [[likely]] {
        continue;
      }

      const auto index = _layers[y][x];
      if (index >= _tile_sources.size()) [[unlikely]] {
        continue;
      }

      const auto& source = _tile_sources[index];
      const geometry::rectangle destination{ {dest_x, dest_y}, {_tilesize, _tilesize} };

      _tileset->draw(source, destination);
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  _target = std::move(object);
}
