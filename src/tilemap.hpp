#pragma once

#include "common.hpp"

#include "object.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

namespace framework {
struct transaction final {
  std::vector<std::string> path;
  uint16_t delay;

  friend void from_json(const nlohmann::json& j, transaction& t) {
    j.at("path").get_to(t.path);
    j.at("delay").get_to(t.delay);
  }
};

class tilemap final {
  public:
    tilemap() = delete;
    explicit tilemap(
      geometry::size size,
      std::shared_ptr<resourcemanager> resourcemanager,
      const std::string& name
    );
    ~tilemap() = default;

    void update(float_t delta) noexcept;
    void draw() const noexcept;
    void set_target(std::shared_ptr<object> object);

    std::vector<std::string> under() const noexcept;

  private:
    float_t _size;
    float_t _height;
    float_t _width;
    std::shared_ptr<graphics::pixmap> _pixmap;
    std::vector<geometry::rectangle> _sources;
    std::vector<std::string> _labels;
    std::vector<bool> _visibles;
    std::vector<std::vector<uint8_t>> _layers;
    std::vector<transaction> _transactions;
    std::shared_ptr<object> _target;
    geometry::rectangle _view;

    uint64_t _last_tick{0};
    size_t _current_transaction{0};
};
}
