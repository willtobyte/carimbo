#pragma once

#include "common.hpp"

struct alignas(16) tile final {
  int32_t x;
  int32_t y;
  uint32_t id;

  friend void from_json(unmarshal::value json, tile& out);
};

struct alignas(64) layer final {
  bool collider;
  std::string name;
  std::vector<tile> tiles;

  friend void from_json(unmarshal::value json, layer& out);
};

struct alignas(16) tile_uv final {
  float u0, v0, u1, v1;
};

struct alignas(64) layer_grid final {
  std::vector<uint32_t> tiles;
  bool collider;
};

class tilemap final {
public:
  tilemap(std::string_view name, std::shared_ptr<resourcemanager> resourcemanager); 

  friend void from_json(unmarshal::document& document, tilemap& out);

  void set_viewport(const quad& value) noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

private:
  int32_t _tile_size;
  int32_t _width;
  int32_t _height;
  float _tile_size_f;
  float _inv_tile_size;
  quad _viewport;

  std::vector<tile_uv> _uv_table;
  std::vector<layer_grid> _grids;
  std::vector<layer> _layers;

  std::vector<SDL_Vertex> _vertices;
  std::vector<int32_t> _indices;

  std::shared_ptr<pixmap> _atlas;
  std::shared_ptr<renderer> _renderer;
};
