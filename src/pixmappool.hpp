#pragma once

#include "common.hpp"

namespace graphics {
using pixmap_ptr = std::shared_ptr<pixmap>;

class pixmappool {
public:
  explicit pixmappool(std::shared_ptr<renderer> renderer) noexcept;
  ~pixmappool() noexcept = default;

  std::shared_ptr<pixmap> get(const std::string &filename);

  void flush() noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unordered_map<std::string, pixmap_ptr> _pool;
};
}
