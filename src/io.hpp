#pragma once

#include "common.hpp"

namespace storage {
class io {
public:
  io() = delete;
  ~io() = delete;

  static std::vector<uint8_t> read(std::string_view filename);
};
}
