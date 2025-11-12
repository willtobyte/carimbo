#pragma once

#include "common.hpp"

namespace storage {
class filesystem final {
public:
  static void mount(const std::string_view filename, const std::string_view mountpoint);

private:
  filesystem() = delete;
  ~filesystem() = delete;
};
}
