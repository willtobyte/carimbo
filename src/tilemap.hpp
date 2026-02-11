#pragma once

#include "common.hpp"

#include "physics.hpp"

struct alignas(16) tile_uv final {
  float u0, v0, u1, v1;
};

struct alignas(64) grid final {
  std::vector<uint32_t> tiles;
  bool collider;

  grid(unmarshal::json node) noexcept
      : collider(node["collider"].get(false)) {
    const auto tilesnode = node["tiles"];
    tiles.reserve(tilesnode.size());
    tilesnode.foreach([this](unmarshal::json tile) {
      tiles.emplace_back(tile.get<uint32_t>());
    });
  }
};

class tilemap final {
public:
  tilemap(std::string_view name, physics::world& world);

  void set_viewport(const quad& value);

  void update(float delta);

  void draw() const noexcept;

  [[nodiscard]] std::span<const grid> grids() const noexcept;
  [[nodiscard]] int32_t width() const noexcept;
  [[nodiscard]] int32_t height() const noexcept;
  [[nodiscard]] float tile_size() const noexcept;

private:
  int32_t _width;
  int32_t _height;
  float _tile_size;
  float _inv_tile_size;
  bool _dirty{true};

  quad _viewport;
  std::vector<tile_uv> _uv_table;
  std::vector<grid> _grids;

  std::vector<SDL_Vertex> _vertices;
  std::vector<int32_t> _indices;

  std::shared_ptr<pixmap> _atlas;

  std::vector<physics::body> _bodies;
};
