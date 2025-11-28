#pragma once

#include "common.hpp"

class filesystem final {
public:
  static void mount(const std::string_view filename, const std::string_view mountpoint) noexcept;

private:
  filesystem() = delete;
  ~filesystem() = delete;
};
