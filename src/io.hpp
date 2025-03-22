#pragma once

#include "common.hpp"

namespace storage {
class io {
public:
  io() = delete;
  ~io() = delete;

  static std::vector<char> read(const std::string &filename);
};
}
