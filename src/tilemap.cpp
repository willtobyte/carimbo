#include "tilemap.hpp"

#include "pixmap.hpp"
#include "renderer.hpp"

tilemap::tilemap(std::string_view name, std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {
  auto json = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));

  _tile_size = json["tile_size"].get<float>();
  _width = json["width"].get<int32_t>();
  _height = json["height"].get<int32_t>();

  json["layers"].foreach([this](unmarshal::json node) {
    _grids.emplace_back(std::move(node));
  });

  _atlas = std::make_shared<pixmap>(_renderer, std::format("blobs/tilemaps/{}.png", name));
  _tile_size = static_cast<float>(_tile_size);
  _inv_tile_size = 1.0f / _tile_size;

  const auto tiles_per_row = _atlas->width() / static_cast<int32_t>(_tile_size);
  const auto tiles_per_column = _atlas->height() / static_cast<int32_t>(_tile_size);

  {
    const auto atlas_width = static_cast<float>(_atlas->width());
    const auto atlas_height = static_cast<float>(_atlas->height());
    const auto u_scale = _tile_size / atlas_width;
    const auto v_scale = _tile_size / atlas_height;

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

void tilemap::set_viewport(const quad& value) {
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

void tilemap::update([[maybe_unused]] float delta) {
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

  constexpr SDL_FColor white{1.0f, 1.0f, 1.0f, 1.0f};

  for (const auto& grid : _grids) {
    const auto* __restrict tiles = grid.tiles.data();

    auto row_offset = start_row * _width;
    auto dy_base = static_cast<float>(start_row) * _tile_size - _viewport.y;

    for (auto row = start_row; row <= end_row; ++row, row_offset += _width, dy_base += _tile_size) {
      for (auto column = start_column; column <= end_column; ++column) {
        const auto tile_id = tiles[row_offset + column];

        if (tile_id == 0) [[likely]] {
          continue;
        }

        const auto& uv = _uv_table[tile_id - 1];

        const auto dx = static_cast<float>(column) * _tile_size - _viewport.x;

        const auto base = static_cast<int32_t>(_vertices.size());

        _vertices.emplace_back(SDL_Vertex{{dx, dy_base}, white, {uv.u0, uv.v0}});
        _vertices.emplace_back(SDL_Vertex{{dx + _tile_size, dy_base}, white, {uv.u1, uv.v0}});
        _vertices.emplace_back(SDL_Vertex{{dx + _tile_size, dy_base + _tile_size}, white, {uv.u1, uv.v1}});
        _vertices.emplace_back(SDL_Vertex{{dx, dy_base + _tile_size}, white, {uv.u0, uv.v1}});

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

std::span<const grid> tilemap::grids() const noexcept { return _grids; }

int32_t tilemap::width() const noexcept { return _width; }

int32_t tilemap::height() const noexcept { return _height; }

float tilemap::tile_size() const noexcept { return _tile_size; }
