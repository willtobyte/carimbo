#pragma once

#include "common.hpp"

struct alignas(16) tile_uv final {
  float u0, v0, u1, v1;
};

struct alignas(64) grid final {
  std::vector<uint32_t> tiles;
  bool collider;

  friend void from_json(unmarshal::value json, grid& out);
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
  bool _dirty{true};

  std::vector<tile_uv> _uv_table;
  std::vector<grid> _grids;

  std::vector<SDL_Vertex> _vertices;
  std::vector<int32_t> _indices;

  std::shared_ptr<pixmap> _atlas;
  std::shared_ptr<renderer> _renderer;
};
