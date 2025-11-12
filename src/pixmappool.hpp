#pragma once

#include "common.hpp"

namespace graphics {
using pixmap_ptr = std::shared_ptr<pixmap>;

class pixmappool final {
public:
  explicit pixmappool(std::shared_ptr<renderer> renderer);
  ~pixmappool() = default;

  std::shared_ptr<pixmap> get(std::string_view filename);

  void flush();

#ifndef NDEBUG
  void debug() const;
#endif

private:
  std::shared_ptr<renderer> _renderer;

  std::unordered_map<std::string, pixmap_ptr, string_hash, std::equal_to<>> _pool;
};
}