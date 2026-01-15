#include "tilemap.hpp"

#include "physics.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

tilemap::tilemap(std::string_view name, std::shared_ptr<renderer> renderer, b2WorldId world)
    : _renderer(std::move(renderer)) {
  auto json = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));

  _tile_size = json["tile_size"].get<float>();
  _width = json["width"].get<int32_t>();
  _height = json["height"].get<int32_t>();

  const auto layers = json["layers"];
  _grids.reserve(layers.size());
  layers.foreach([this](unmarshal::json node) {
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

  const auto half = _tile_size * 0.5f;
  const auto sdef = b2DefaultShapeDef();
  const auto width = static_cast<size_t>(_width);
  const auto height = static_cast<size_t>(_height);
  const auto total = width * height;
  std::vector<uint8_t> visited(total);
  _bodies.reserve(total / 2);

  for (const auto& grid : _grids) {
    if (!grid.collider) continue;

    std::memset(visited.data(), 0, total);
    const auto* __restrict tiles = grid.tiles.data();
    auto* __restrict visited_data = visited.data();

    for (size_t row = 0; row < height; ++row) {
      const auto row_offset = row * width;

      for (size_t column = 0; column < width; ++column) {
        const auto index = row_offset + column;
        if (tiles[index] == 0 || visited_data[index]) [[likely]] continue;

        auto run_width = size_t{1};
        while (column + run_width < width && tiles[index + run_width] != 0 && !visited_data[index + run_width]) {
          ++run_width;
        }

        auto run_height = size_t{1};
        while (row + run_height < height) {
          const auto check_offset = (row + run_height) * width + column;
          __builtin_prefetch(tiles + check_offset + width);
          __builtin_prefetch(visited_data + check_offset + width);

          auto valid = true;
          for (size_t dx = 0; dx < run_width; ++dx) {
            if (tiles[check_offset + dx] == 0 || visited_data[check_offset + dx]) {
              valid = false;
              break;
            }
          }

          if (!valid) break;
          ++run_height;
        }

        for (size_t dy = 0; dy < run_height; ++dy) {
          std::memset(visited_data + (row + dy) * width + column, 1, run_width);
        }

        const auto half_w = static_cast<float>(run_width) * half;
        const auto half_h = static_cast<float>(run_height) * half;
        const auto position = b2Vec2{
          static_cast<float>(column) * _tile_size + half_w,
          static_cast<float>(row) * _tile_size + half_h
        };

        const auto poly = b2MakeBox(half_w, half_h);
        const auto body = physics::make_static_body(world, position);
        b2CreatePolygonShape(body, &sdef, &poly);
        _bodies.emplace_back(body);
      }
    }
  }
}

tilemap::~tilemap() noexcept {
  for (auto& body : _bodies) {
    physics::destroy_body(body);
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
