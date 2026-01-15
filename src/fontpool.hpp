#pragma once

#include "common.hpp"

class fontpool final {
public:
  explicit fontpool(std::shared_ptr<renderer> renderer);
  ~fontpool() noexcept = default;

  std::shared_ptr<font> get(std::string_view family);

private:
  std::shared_ptr<renderer> _renderer;
  boost::unordered_flat_map<std::string, std::shared_ptr<font>, transparent_string_hash, std::equal_to<>> _fonts;
};
