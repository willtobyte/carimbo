#pragma once

#include "common.hpp"

namespace storage {
class filesystem final {
public:
  static void mount(const std::string& filename, const std::string& mountpoint);

private:
  filesystem() = delete;
  ~filesystem() = delete;
};
}
