#pragma once

#include "common.hpp"
#include "object.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

namespace framework {
class tilemap final {
  public:
    tilemap() = delete;
    explicit tilemap(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<resourcemanager> resourcemanager, const std::string& name);
    ~tilemap() = default;

    void update(float_t delta) noexcept;
    void draw() const noexcept;
    void set_target(std::shared_ptr<object> object);

  private:
    std::shared_ptr<graphics::pixmap> _tileset;
    std::vector<geometry::rectangle> _tilesources;
    float_t _tilesize;
    std::vector<std::vector<uint16_t>> _layers;
    std::shared_ptr<object> _target;
    geometry::rectangle _view;
};
}
