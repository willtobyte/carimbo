#pragma once

#include "common.hpp"

namespace framework {
class scene {
public:
  scene() = delete;
  scene(std::shared_ptr<graphics::pixmap> background, std::unordered_map<std::string, std::shared_ptr<object>> objects, geometry::size size) noexcept;
  ~scene() noexcept = default;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

private:
  std::shared_ptr<graphics::pixmap> _background;
  std::unordered_map<std::string, std::shared_ptr<object>> _objects;
  geometry::size _size;
};
}
