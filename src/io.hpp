#pragma once

#include "common.hpp"

namespace storage {
class io {
public:
  io() = delete;
  ~io() = delete;

  static std::span<const std::byte> read(const std::string &filename);
};
}
