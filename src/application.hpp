#pragma once

#include "common.hpp"

class application final {
public:
  application() = default;
  ~application() = default;

  [[nodiscard]] int run();
};
