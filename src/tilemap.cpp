#include "tilemap.hpp"

void from_json(unmarshal::value json, tile& out) {
  out.x = unmarshal::get<float>(json, "x");
  out.y = unmarshal::get<float>(json, "y");
  out.id = unmarshal::get<uint64_t>(json, "id");
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
  out._tile_size = unmarshal::get<int16_t>(document, "tile_size");
  out._map_width = unmarshal::get<int64_t>(document, "map_width");
  out._map_height = unmarshal::get<int64_t>(document, "map_height");

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
  _tiles_per_row = static_cast<uint32_t>(_atlas->width() / _tile_size);

  const auto max_tiles = static_cast<size_t>(static_cast<size_t>(_map_width) * static_cast<size_t>(_map_height) * _layers.size());
  _vertices.reserve(max_tiles * 4);
  _indices.reserve(max_tiles * 6);
}

void tilemap::set_viewport(const quad& value) noexcept {
  _viewport = value;
}

void tilemap::update([[maybe_unused]] float delta) noexcept {
  _vertices.clear();
  _indices.clear();

  const auto start_col = static_cast<int32_t>(_viewport.x / _tile_size);
  const auto start_row = static_cast<int32_t>(_viewport.y / _tile_size);
  const auto end_col = static_cast<int32_t>((_viewport.x + _viewport.w) / _tile_size) + 1;
  const auto end_row = static_cast<int32_t>((_viewport.y + _viewport.h) / _tile_size) + 1;

  const auto u_scale = static_cast<float>(_tile_size) / static_cast<float>(_atlas->width());
  const auto v_scale = static_cast<float>(_tile_size) / static_cast<float>(_atlas->height());

  for (const auto& layer : _layers) {
    for (const auto& tile : layer.tiles) {
      const auto col = static_cast<int32_t>(tile.x);
      const auto row = static_cast<int32_t>(tile.y);

      if (col < start_col || col > end_col || row < start_row || row > end_row) [[unlikely]] {
        continue;
      }

      const auto dx = static_cast<float>(col * _tile_size) - _viewport.x;
      const auto dy = static_cast<float>(row * _tile_size) - _viewport.y;
      const auto dw = static_cast<float>(_tile_size);
      const auto dh = static_cast<float>(_tile_size);

      const auto tile_col = static_cast<uint32_t>(tile.id % _tiles_per_row);
      const auto tile_row = static_cast<uint32_t>(tile.id / _tiles_per_row);
      const auto u0 = static_cast<float>(tile_col) * u_scale;
      const auto v0 = static_cast<float>(tile_row) * v_scale;
      const auto u1 = u0 + u_scale;
      const auto v1 = v0 + v_scale;

      const auto base = static_cast<int32_t>(_vertices.size());

      constexpr SDL_FColor white{1.f, 1.f, 1.f, 1.f};
      _vertices.push_back({{dx, dy}, white, {u0, v0}});
      _vertices.push_back({{dx + dw, dy}, white, {u1, v0}});
      _vertices.push_back({{dx + dw, dy + dh}, white, {u1, v1}});
      _vertices.push_back({{dx, dy + dh}, white, {u0, v1}});

      _indices.push_back(base);
      _indices.push_back(base + 1);
      _indices.push_back(base + 2);
      _indices.push_back(base);
      _indices.push_back(base + 2);
      _indices.push_back(base + 3);
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

