#pragma once

#include "common.hpp"

struct alignas(16) tile final {
  float x;
  float y;
  uint64_t id;

  friend void from_json(unmarshal::value json, tile& out);
};

struct alignas(64) layer final {
  bool collider;
  std::string name;
  std::vector<tile> tiles;

  friend void from_json(unmarshal::value json, layer& out);
};

class tilemap final {
public:
  tilemap(std::string_view name, std::shared_ptr<pixmappool> pixmappool);

  friend void from_json(unmarshal::document& document, tilemap& out);

  void set_viewport(const quad& value) noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

private:
  int16_t _tile_size;
  uint32_t _tiles_per_row;
  int64_t _map_width;
  int64_t _map_height;
  quad _viewport;
  std::vector<SDL_Vertex> _vertices;
  std::vector<int32_t> _indices;
  std::vector<layer> _layers;
  std::shared_ptr<pixmap> _atlas;
};
