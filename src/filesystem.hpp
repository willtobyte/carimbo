#pragma once

#include "common.hpp"

namespace storage {
class filesystem final {
public:
  static void mount(std::string_view filename, std::string_view mountpoint);

private:
  filesystem() = delete;
  ~filesystem() = delete;
};
}
