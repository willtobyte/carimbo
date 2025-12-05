#pragma once

#include "common.hpp"

  class font;
  class pixmappool;
  class renderer;

class fontfactory final {
public:
  fontfactory() = delete;
  explicit fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool);
  ~fontfactory() = default;

  std::shared_ptr<font> get(std::string_view family) noexcept;

  void flush();

#ifndef NDEBUG
  void debug() const;
#endif

private:
  boost::unordered_flat_map<std::string, std::shared_ptr<font>, transparent_string_hash, std::equal_to<>> _pool;
  std::shared_ptr<renderer> _renderer;
  std::shared_ptr<pixmappool> _pixmappool;
};
