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

  _view = {
    .0f,
    .0f,
    width,
    height
  };

  // const auto& filename = fmt::format("tilemaps/{}.json", name);
  // const auto& buffer = storage::io::read(filename);
  // const auto& j = nlohmann::json::parse(buffer);

  _tilesize = 16.f;

  _tileset = resourcemanager->pixmappool()->get("blobs/tilesets/0.png");

  _layers = {
    {1, 0, 0, 1},
    {1, 0, 1, 0},
    {0, 1, 0, 1},
    {1, 0, 1, 0}
  };
  // UNUSED(j);
}

void tilemap::update(float_t delta) noexcept {
  UNUSED(delta);

  if (!_target || !_tileset) {
    return;
  }

  const constexpr auto smooth = 3.0f;
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
  if (!_tileset) {
    return;
  }

  const auto tiles_per_row = static_cast<uint32_t>(_tileset->width() / _tilesize);

  for (const auto& layer : _layers) {
    const size_t map_height_tiles = _layers.size();
    const size_t map_width_tiles = layer.size();

    for (size_t y = 0; y < map_height_tiles; ++y) {
      for (size_t x = 0; x < map_width_tiles; ++x) {
        const auto index = _layers[y][x];
        const auto src_x = static_cast<float_t>(index % tiles_per_row) * _tilesize;
        const auto src_y = (static_cast<float_t>(index) / static_cast<float_t>(tiles_per_row)) * _tilesize;

        const geometry::rectangle source{
          {src_x, src_y},
          {_tilesize, _tilesize}
        };

        const geometry::rectangle destination{
          {x * _tilesize, y * _tilesize},
          {_tilesize, _tilesize}
        };

        _tileset->draw(source, destination);
      }
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  _target = std::move(object);
}
