#pragma once

#include "common.hpp"

class application final {
public:
  application(int argc, char **argv);
  ~application() = default;

  [[nodiscard]] int run();
};
