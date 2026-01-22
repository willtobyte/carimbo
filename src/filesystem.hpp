#pragma once

#include "common.hpp"

class filesystem final {
public:
  static void mount(std::string_view filename, std::string_view mountpoint) noexcept;

private:
  filesystem() = delete;
  ~filesystem() = delete;
};
