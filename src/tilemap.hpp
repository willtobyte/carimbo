#pragma once

#include "common.hpp"

#include "object.hpp"
#include "pixmap.hpp"
#include "resourcemanager.hpp"

namespace framework {
class tilemap {
  public:
    tilemap() = delete;
    explicit tilemap(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<resourcemanager> resourcemanager, const std::string &name) noexcept;
    ~tilemap() noexcept = default;

    void update(float_t delta) noexcept;

    void draw() const noexcept;

    void set_target(std::shared_ptr<object> object) noexcept;

  private:
    std::shared_ptr<resourcemanager> _resourcemanager;
    std::shared_ptr<graphics::pixmap> _tileset;
    uint8_t _qsize;

    std::shared_ptr<object> _target;
    geometry::rectangle _view;
};
}
