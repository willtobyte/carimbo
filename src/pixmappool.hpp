#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "renderer.hpp"

namespace graphics {
using pixmap_ptr = std::shared_ptr<pixmap>;

class pixmappool final {
public:
  explicit pixmappool(std::shared_ptr<renderer> renderer) noexcept;
  ~pixmappool() noexcept = default;

  std::shared_ptr<pixmap> get(const std::string& filename);

  void flush() noexcept;

  void set_loop(std::function<void()> fn) noexcept;

#ifdef DEBUG
  void debug() const noexcept;
#endif

private:
  std::shared_ptr<renderer> _renderer;

  std::unordered_map<
    std::string,
    pixmap_ptr,
    std::hash<std::string>,
    std::equal_to<std::string>
  > _pool;

  std::function<void()> _loop;
};
}
