#pragma once

#include "common.hpp"

#include "font.hpp"
#include "renderer.hpp"
#include "pixmappool.hpp"

namespace graphics {
class fontfactory final {
public:
  fontfactory() = delete;
  explicit fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool);
  ~fontfactory() = default;

  std::shared_ptr<font> get(const std::string &family);

  void flush();

private:
  std::unordered_map<std::string, std::shared_ptr<font>> _pool;
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<pixmappool> _pixmappool;
};
}
