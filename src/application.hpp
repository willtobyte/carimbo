#pragma once

#include "common.hpp"

namespace framework {
class application final {
public:
  application(int argc, char **argv);
  ~application() = default;

  application(const application&) = delete;
  application& operator=(const application&) = delete;
  application(application&&) = delete;
  application& operator=(application&&) = delete;

  int32_t run();
};
}
