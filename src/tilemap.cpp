#include "tilemap.hpp"

void from_json(unmarshal::value json, tile& out) {
  out.x = unmarshal::get<int32_t>(json, "x");
  out.y = unmarshal::get<int32_t>(json, "y");
  out.id = unmarshal::get<uint32_t>(json, "id");
}

void from_json(unmarshal::value json, layer& out) {
  out.collider = unmarshal::get<bool>(json, "collider");
  out.name = unmarshal::get<std::string_view>(json, "name");

  out.tiles.clear();
  for (auto element : json["tiles"].get_array()) {
    out.tiles.emplace_back(unmarshal::make<tile>(element));
  }
}

void from_json(unmarshal::document& document, tilemap& out) {
  out._tile_size = unmarshal::get<int32_t>(document, "tile_size");
  out._width = unmarshal::get<int32_t>(document, "width");
  out._height = unmarshal::get<int32_t>(document, "height");

  out._layers.clear();
  for (auto element : document["layers"].get_array()) {
    out._layers.emplace_back(unmarshal::make<layer>(element));
  }
}

tilemap::tilemap(std::string_view name, std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool)
    : _renderer(std::move(renderer)) {
  const auto document = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));
  from_json(*document, *this);

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

  {
    const auto grid_size = static_cast<size_t>(_width) * static_cast<size_t>(_height);

    _grids.resize(_layers.size());

    for (size_t i = 0; i < _layers.size(); ++i) {
      auto& grid = _grids[i];
      grid.collider = _layers[i].collider;
      grid.tiles.assign(grid_size, 0);

      for (const auto& tile : _layers[i].tiles) {
        const auto index = static_cast<size_t>(tile.y) * static_cast<size_t>(_width) + static_cast<size_t>(tile.x);
        grid.tiles[index] = tile.id + 1;
      }
    }
  }

  _layers.clear();
  _layers.shrink_to_fit();
}

void tilemap::set_viewport(const quad& value) noexcept {
  _viewport = value;

  const auto tiles_x = static_cast<size_t>(value.w * _inv_tile_size) + 2;
  const auto tiles_y = static_cast<size_t>(value.h * _inv_tile_size) + 2;
  const auto max_tiles = tiles_x * tiles_y * _grids.size();

  _vertices.reserve(max_tiles * 4);
  _indices.reserve(max_tiles * 6);
}

void tilemap::update([[maybe_unused]] float delta) noexcept {
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

    for (auto row = start_row; row <= end_row; ++row) {
      const auto row_offset = row * width;
      const auto dy_base = static_cast<float>(row) * tile_size - viewport_y;

      for (auto column = start_column; column <= end_column; ++column) {
        const auto tile_id = tiles[row_offset + column];

        if (tile_id == 0) [[likely]] {
          continue;
        }

        const auto& uv = _uv_table[tile_id - 1];

        const auto dx = static_cast<float>(column) * tile_size - viewport_x;
        const auto dy = dy_base;

        const auto base = static_cast<int32_t>(_vertices.size());

        _vertices.push_back({{dx, dy}, white, {uv.u0, uv.v0}});
        _vertices.push_back({{dx + tile_size, dy}, white, {uv.u1, uv.v0}});
        _vertices.push_back({{dx + tile_size, dy + tile_size}, white, {uv.u1, uv.v1}});
        _vertices.push_back({{dx, dy + tile_size}, white, {uv.u0, uv.v1}});

        _indices.push_back(base);
        _indices.push_back(base + 1);
        _indices.push_back(base + 2);
        _indices.push_back(base);
        _indices.push_back(base + 2);
        _indices.push_back(base + 3);
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
