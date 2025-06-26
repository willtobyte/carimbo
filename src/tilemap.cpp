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

  const float_t width = lw / sx;
  const float_t height = lh / sy;

  _view = { .0f, .0f, width, height };
  _tilesize = 16.f;

  _tileset = resourcemanager->pixmappool()->get("blobs/tilesets/0.png");

  _layers = {
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 2, 2, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 2, 2, 2, 2, 1},
  };

  const uint32_t tiles_per_row = _tileset->width() / static_cast<uint32_t>(_tilesize);
  static constexpr uint32_t max_index = 255;

  _tile_sources.resize(max_index + 1);
  _tile_sources[0] = geometry::rectangle{{-1.f, -1.f}, {0.f, 0.f}};

  for (uint32_t i = 1; i <= max_index; ++i) {
    const uint32_t zbi = i - 1;
    const float_t src_x = (zbi % tiles_per_row) * _tilesize;
    const float_t src_y = (zbi / static_cast<float_t>(tiles_per_row)) * _tilesize;

    _tile_sources[i] = geometry::rectangle{{src_x, src_y}, {_tilesize, _tilesize}};
  }
}

void tilemap::update(float_t delta) noexcept {
  UNUSED(delta);

  if (!_target || !_tileset) [[unlikely]] {
    return;
  }

  const auto position = _target->position();
  const float_t vw = _view.width();
  const float_t vh = _view.height();

  _view.set_position(position.x() - vw * 0.5f, position.y() - vh * 0.5f);
}

void tilemap::draw() const noexcept {
  if (!_tileset) [[unlikely]] {
    return;
  }

  const float view_x0 = _view.x();
  const float view_y0 = _view.y();
  const float view_x1 = view_x0 + _view.width();
  const float view_y1 = view_y0 + _view.height();

  const size_t map_height_tiles = _layers.size();

  for (size_t y = 0; y < map_height_tiles; ++y) {
    const size_t map_width_tiles = _layers[y].size();

    for (size_t x = 0; x < map_width_tiles; ++x) {
      const float tile_x = x * _tilesize;
      const float tile_y = y * _tilesize;

      if (tile_x + _tilesize < view_x0) [[likely]] continue;
      if (tile_x > view_x1) [[likely]] continue;
      if (tile_y + _tilesize < view_y0) [[likely]] continue;
      if (tile_y > view_y1) [[likely]] continue;

      const uint32_t index = _layers[y][x];
      if (!index || index >= _tile_sources.size()) [[unlikely]] continue;

      const auto& source = _tile_sources[index];
      const float screen_x = tile_x - view_x0;
      const float screen_y = tile_y - view_y0;

      const geometry::rectangle destination{{screen_x, screen_y}, {_tilesize, _tilesize}};
      _tileset->draw(source, destination);
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  if (!object) [[unlikely]] return;
  _target = std::move(object);
}
