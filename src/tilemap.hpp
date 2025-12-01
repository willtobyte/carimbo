#pragma once

#include "common.hpp"

struct tile final {
  float x, y;
  uint64_t pixmap;
};

struct tileset final {
  float w, h;
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
