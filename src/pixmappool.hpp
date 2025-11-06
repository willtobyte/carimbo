#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "renderer.hpp"

namespace graphics {
using pixmap_ptr = std::shared_ptr<pixmap>;

class pixmappool final {
public:
  explicit pixmappool(std::shared_ptr<renderer> renderer);
  ~pixmappool() = default;

  std::shared_ptr<pixmap> get(const std::string& filename);

  void flush();

#ifndef NDEBUG
  void debug() const;
#endif

private:
  std::shared_ptr<renderer> _renderer;

  std::unordered_map<
    std::string,
    pixmap_ptr,
    std::hash<std::string>,
    std::equal_to<std::string>
  > _pool;
};
}
