#pragma once

#include "common.hpp"
#include "io.hpp"
#include "pixmap.hpp"
#include "pixmappool.hpp"

struct alignas(16) tile final {
  float x;
  float y;
  uint32_t id;

  friend void from_json(const nlohmann::json& j, tile& t) {
    j.at("x").get_to(t.x);
    j.at("y").get_to(t.y);
    t.id = static_cast<uint32_t>(std::stoul(j.at("id").get<std::string>()));
  }
};

struct alignas(64) layer final {
  bool collider;
  std::string name;
  std::vector<tile> tiles;

  friend void from_json(const nlohmann::json& j, layer& l) {
    j.at("collider").get_to(l.collider);
    j.at("name").get_to(l.name);
    j.at("tiles").get_to(l.tiles);
  }
};

class tilemap final {
public:
  tilemap(std::string_view name, std::shared_ptr<pixmappool> pixmappool) {

    const auto buffer = io::read(std::format("tilemaps/{}.json", name));
    const auto j = nlohmann::json::parse(buffer);
    from_json(j, *this);

    atlas = pixmappool->get(std::format("blobs/tilemaps/{}.png", name));
  }

  friend void from_json(const nlohmann::json& j, tilemap& t) {
    j.at("tileSize").get_to(t.tile_size);
    j.at("mapWidth").get_to(t.map_width);
    j.at("mapHeight").get_to(t.map_height);
    j.at("layers").get_to(t.layers);
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
