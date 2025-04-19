#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "resourcemanager.hpp"

namespace framework {
class tilemap {
  public:
    explicit tilemap(std::shared_ptr<resourcemanager> resourcemanager, const std::string &name) noexcept;
    tilemap() noexcept = default;

  private:
    std::shared_ptr<resourcemanager> _resourcemanager;
    std::shared_ptr<graphics::pixmap> _tileset;
};
}
