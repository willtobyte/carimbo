#pragma once

#include "common.hpp"

#include "font.hpp"
#include "renderer.hpp"

namespace graphics {
class fontfactory {
public:
  fontfactory() noexcept = delete;
  explicit fontfactory(std::shared_ptr<renderer> renderer) noexcept;
  ~fontfactory() noexcept = default;

  std::shared_ptr<font> get(const std::string &family);

  void flush() noexcept;

private:
  std::unordered_map<std::string, std::shared_ptr<font>> _pool;
  std::shared_ptr<renderer> _renderer;
};
}
