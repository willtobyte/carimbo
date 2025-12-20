#include "tilemap.hpp"

void from_json(unmarshal::value json, grid& out) {
  out.collider = unmarshal::get<bool>(json, "collider");

  auto array = unmarshal::get_array(json, "tiles");
  out.tiles.reserve(unmarshal::count(array));

  for (auto element : array) {
    out.tiles.emplace_back(static_cast<uint32_t>(unmarshal::get<uint64_t>(element)));
  }
}

void from_json(unmarshal::document& document, tilemap& out) {
  out._tile_size = unmarshal::get<int32_t>(document, "tile_size");
  out._width = unmarshal::get<int32_t>(document, "width");
  out._height = unmarshal::get<int32_t>(document, "height");

  auto layers = unmarshal::get_array(document, "layers");
  out._grids.reserve(unmarshal::count(layers));

  for (auto element : layers) {
    out._grids.emplace_back(unmarshal::make<grid>(element));
  }
}

tilemap::tilemap(std::string_view name, std::shared_ptr<resourcemanager> resourcemanager)
    : _renderer(resourcemanager->renderer()) {
  auto document = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));
  from_json(*document, *this);

  const auto pixmappool = resourcemanager->pixmappool();

  _atlas = pixmappool->get(std::format("blobs/tilemaps/{}.png", name));

  _tile_size_f = static_cast<float>(_tile_size);
  _inv_tile_size = 1.0f / _tile_size_f;

  const auto tiles_per_row = _atlas->width() / _tile_size;
  const auto tiles_per_column = _atlas->height() / _tile_size;

  {
    const auto atlas_width = static_cast<float>(_atlas->width());
    const auto atlas_height = static_cast<float>(_atlas->height());
    const auto u_scale = _tile_size_f / atlas_width;
    const auto v_scale = _tile_size_f / atlas_height;

    const auto total_tiles = static_cast<size_t>(tiles_per_row) * static_cast<size_t>(tiles_per_column);
    _uv_table.resize(total_tiles);

    for (size_t id = 0; id < total_tiles; ++id) {
      const auto tile_column = static_cast<int32_t>(id % static_cast<size_t>(tiles_per_row));
      const auto tile_row = static_cast<int32_t>(id / static_cast<size_t>(tiles_per_row));

      auto& uv = _uv_table[id];
      uv.u0 = static_cast<float>(tile_column) * u_scale;
      uv.v0 = static_cast<float>(tile_row) * v_scale;
      uv.u1 = uv.u0 + u_scale;
      uv.v1 = uv.v0 + v_scale;
    }
  }

}

void tilemap::set_viewport(const quad& value) noexcept {
  if (_viewport == value) [[likely]] {
    return;
  }

  _viewport = value;
  _dirty = true;

  const auto tiles_x = static_cast<size_t>(value.w * _inv_tile_size) + 2;
  const auto tiles_y = static_cast<size_t>(value.h * _inv_tile_size) + 2;
  const auto max_tiles = tiles_x * tiles_y * _grids.size();

  _vertices.reserve(max_tiles * 4);
  _indices.reserve(max_tiles * 6);
}

void tilemap::update([[maybe_unused]] float delta) noexcept {
  if (!_dirty) [[likely]] {
    return;
  }

  _dirty = false;

  _vertices.clear();
  _indices.clear();

  const auto start_column = std::max(0, static_cast<int32_t>(_viewport.x * _inv_tile_size));
  const auto start_row = std::max(0, static_cast<int32_t>(_viewport.y * _inv_tile_size));
  const auto end_column = std::min(_width - 1, static_cast<int32_t>((_viewport.x + _viewport.w) * _inv_tile_size) + 1);
  const auto end_row = std::min(_height - 1, static_cast<int32_t>((_viewport.y + _viewport.h) * _inv_tile_size) + 1);

  if (start_column > end_column || start_row > end_row) [[unlikely]] {
    return;
  }

  const auto viewport_x = _viewport.x;
  const auto viewport_y = _viewport.y;
  const auto tile_size = _tile_size_f;
  const auto width = _width;

  constexpr SDL_FColor white{1.0f, 1.0f, 1.0f, 1.0f};

  for (const auto& grid : _grids) {
    const auto* __restrict tiles = grid.tiles.data();

    auto row_offset = start_row * width;
    auto dy_base = static_cast<float>(start_row) * tile_size - viewport_y;

    for (auto row = start_row; row <= end_row; ++row, row_offset += width, dy_base += tile_size) {
      for (auto column = start_column; column <= end_column; ++column) {
        const auto tile_id = tiles[row_offset + column];

        if (tile_id == 0) [[likely]] {
          continue;
        }

        const auto& uv = _uv_table[tile_id - 1];

        const auto dx = static_cast<float>(column) * tile_size - viewport_x;

        const auto base = static_cast<int32_t>(_vertices.size());

        _vertices.emplace_back(SDL_Vertex{{dx, dy_base}, white, {uv.u0, uv.v0}});
        _vertices.emplace_back(SDL_Vertex{{dx + tile_size, dy_base}, white, {uv.u1, uv.v0}});
        _vertices.emplace_back(SDL_Vertex{{dx + tile_size, dy_base + tile_size}, white, {uv.u1, uv.v1}});
        _vertices.emplace_back(SDL_Vertex{{dx, dy_base + tile_size}, white, {uv.u0, uv.v1}});

        _indices.emplace_back(base);
        _indices.emplace_back(base + 1);
        _indices.emplace_back(base + 2);
        _indices.emplace_back(base);
        _indices.emplace_back(base + 2);
        _indices.emplace_back(base + 3);
      }
    }
  }
}

void tilemap::draw() const noexcept {
  if (_vertices.empty()) [[unlikely]] {
    return;
  }

  SDL_RenderGeometry(
      *_renderer,
      static_cast<SDL_Texture*>(*_atlas),
      _vertices.data(),
      static_cast<int>(_vertices.size()),
      _indices.data(),
      static_cast<int>(_indices.size())
  );
}
