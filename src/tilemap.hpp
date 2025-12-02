#pragma once

#include "common.hpp"

struct alignas(16) uv final {
  float u0, v0;
  float u1, v1;
};

struct alignas(16) tileset final {
  vec2 size;
  vec2 atlas_size_inv;
  uint32_t tiles_per_row;
};

struct alignas(16) tile final {
  vec2 position;
  uint32_t uv_index;
};

struct alignas(64) layer final {
  std::vector<tile> tiles;
  std::string name;
  bool collider{false};
};

struct alignas(64) tilemap final {
  std::vector<SDL_Vertex> vertices;
  std::vector<int32_t> indices;
  std::vector<uv> table;
  std::vector<layer> layers;
  tileset atlas;
};

// namespace framework {

// struct transaction final {
//   std::vector<std::string> path;
//   uint16_t delay;

//   friend void from_json(const nlohmann::json& j, transaction& t) {
//     j.at("path").get_to(t.path);
//     j.at("delay").get_to(t.delay);
//   }
// };

// class tilemap final {
//   public:
//     tilemap() = delete;
//     explicit tilemap(
//       size size,
//       std::shared_ptr<resourcemanager> resourcemanager,
//       std::string_view name
//     );
//     ~tilemap() = default;

//     void update(float delta);
//     void draw() const;
//     void set_target(std::shared_ptr<object> object);
//     void set_camera(std::shared_ptr<camera> camera);

//     std::vector<std::string> under() const;

//   private:
//     float _size;
//     float _height;
//     float _width;
//     uint64_t _last_tick{0};
//     size_t _current_transaction{0};
//     rectangle _camera;

//     std::shared_ptr<pixmap> _pixmap;
//     std::vector<rectangle> _sources;
//     std::vector<std::string> _labels;
//     std::vector<bool> _visibles;
//     std::vector<std::vector<uint8_t>> _layers;
//     std::vector<transaction> _transactions;
//     std::shared_ptr<object> _target;
//     // std::shared_ptr<camera> _camera;
// };
// }
