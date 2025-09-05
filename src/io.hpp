#pragma once

#include "common.hpp"

namespace storage {
class io final {
public:
  io() = delete;
  ~io() = delete;

  static std::vector<uint8_t> read(const std::string& filename);

  static std::vector<std::string> enumerate(const std::string& directory);
};
}
