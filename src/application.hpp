#pragma once

#include "common.hpp"

namespace framework {
class application final {
public:
  application(int argc, char **argv);
  ~application() = default;

  int run();
};
}
