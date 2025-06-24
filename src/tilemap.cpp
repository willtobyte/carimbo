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
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},
    {1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1},

  };

  const auto tiles_per_row = static_cast<uint32_t>(static_cast<float_t>(_tileset->width()) / _tilesize);

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

  if (!_target || !_tileset) [[unlikely]] {
    return;
  }

  const auto position = _target->position();
  const auto vw = _view.width();
  const auto vh = _view.height();

  std::cout << "Target: (" << position.x() << ", " << position.y() << ")\n";

  _view.set_position({
    position.x() - vw * 0.5f,
    position.y() - vh * 0.5f
  });
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
      const float tile_x = static_cast<float_t>(x) * _tilesize;
      const float tile_y = static_cast<float_t>(y) * _tilesize;

      if (tile_x + _tilesize < view_x0) [[likely]] {
        continue;
      }

      if (tile_x > view_x1) [[likely]] {
        continue;
      }

      if (tile_y + _tilesize < view_y0) [[likely]] {
        continue;
      }

      if (tile_y > view_y1) [[likely]] {
        continue;
      }

      const uint32_t index = _layers[y][x];
      if (index >= _tile_sources.size()) [[unlikely]] {
        continue;
      }

      const auto& source = _tile_sources[index];
      const auto screen_x = tile_x - view_x0;
      const auto screen_y = tile_y - view_y0;

      const geometry::rectangle destination{
        {screen_x, screen_y},
        {_tilesize, _tilesize}
      };

      _tileset->draw(source, destination);
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  if (!object) [[unlikely]] {
    return;
  }

  _target = std::move(object);
}
