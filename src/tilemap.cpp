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
  out.tile_size = unmarshal::get<int16_t>(document, "tile_size");
  out.map_width = unmarshal::get<int64_t>(document, "map_width");
  out.map_height = unmarshal::get<int64_t>(document, "map_height");

  out.layers.clear();
  for (auto element : document["layers"].get_array()) {
    out.layers.emplace_back(unmarshal::make<layer>(element));
  }
}

tilemap::tilemap(std::string_view name, std::shared_ptr<pixmappool> pixmappool) {
  const auto document = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));
  from_json(*document, *this);

  atlas = pixmappool->get(std::format("blobs/tilemaps/{}.png", name));
}

void tilemap::update(float delta) noexcept {
}

void tilemap::draw() const noexcept {
}

