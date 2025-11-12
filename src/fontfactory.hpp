#pragma once

#include "common.hpp"

namespace graphics {
  class font;
  class pixmappool;
  class renderer;
}

namespace graphics {
class fontfactory final {
public:
  fontfactory() = delete;
  explicit fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool);
  ~fontfactory() = default;

  std::shared_ptr<font> get(std::string_view family);

  void flush();

  #ifndef NDEBUG
  void debug() const;
  #endif

private:
  std::unordered_map<std::string, std::shared_ptr<font>, string_hash, std::equal_to<>> _pool;
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<pixmappool> _pixmappool;
  std::function<void()> _loop;
};
}
