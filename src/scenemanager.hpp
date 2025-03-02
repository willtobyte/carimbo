#pragma once

#include "common.hpp"
#include "size.hpp"

namespace framework {
class scenemanager {
public:
  scenemanager(std::shared_ptr<graphics::pixmappool> pixmappool) noexcept;

  void set(const std::string &name) noexcept;

  void grab(const std::string &name) noexcept;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

private:
  std::unordered_map<std::string, std::shared_ptr<entity>> _entities;
  std::shared_ptr<graphics::pixmappool> _pixmappool;
  std::shared_ptr<graphics::pixmap> _background;
  geometry::size _size;
};
}
