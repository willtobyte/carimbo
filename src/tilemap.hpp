#pragma once

#include "common.hpp"

struct alignas(16) tile final {
  float x;
  float y;
  uint32_t id;

  friend void from_json(unmarshal::value json, tile& out) {
    out.x = unmarshal::get<float>(json, "x");
    out.y = unmarshal::get<float>(json, "y");
    out.id = static_cast<uint32_t>(std::stoul(std::string(unmarshal::get<std::string_view>(json, "id"))));
  }
};

struct alignas(64) layer final {
  bool collider;
  std::string name;
  std::vector<tile> tiles;

  friend void from_json(unmarshal::value json, layer& out) {
    out.collider = unmarshal::get<bool>(json, "collider");
    out.name = unmarshal::get<std::string_view>(json, "name");

    out.tiles.clear();
    for (auto element : json["tiles"].get_array()) {
      out.tiles.emplace_back(unmarshal::make<tile>(element));
    }
  }
};

class tilemap final {
public:
  tilemap(std::string_view name, std::shared_ptr<pixmappool> pixmappool) {
    const auto document = unmarshal::parse(io::read(std::format("tilemaps/{}.json", name)));
    from_json(*document, *this);

    atlas = pixmappool->get(std::format("blobs/tilemaps/{}.png", name));
  }

  friend void from_json(unmarshal::document& document, tilemap& out) {
    out.tile_size = unmarshal::get<int32_t>(document, "tileSize");
    out.map_width = unmarshal::get<int32_t>(document, "mapWidth");
    out.map_height = unmarshal::get<int32_t>(document, "mapHeight");

    out.layers.clear();
    for (auto element : document["layers"].get_array()) {
      out.layers.emplace_back(unmarshal::make<layer>(element));
    }
  }

private:
  std::shared_ptr<pixmap> atlas;
  std::vector<SDL_Vertex> vertices;
  std::vector<int32_t> indices;
  std::vector<layer> layers;
  int32_t tile_size;
  int32_t map_width;
  int32_t map_height;
  uint32_t tiles_per_row;
};
