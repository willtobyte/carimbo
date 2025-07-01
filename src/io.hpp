#pragma once

#include "common.hpp"

namespace storage {
class io final {
public:
  io() = delete;
  ~io() = delete;

  static std::vector<std::byte> read(std::string_view filename);

  static std::vector<std::string> enumerate(std::string_view directory);
};
}
