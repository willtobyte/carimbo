#pragma once

#include "common.hpp"

class application final {
public:
  application(int argc, char **argv) noexcept;
  ~application() noexcept = default;

  [[nodiscard]] int run();
};
