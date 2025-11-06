#pragma once

#include "common.hpp"

#include "noncopyable.hpp"

namespace framework {
class application final : private noncopyable {
public:
  application(int argc, char **argv);
  virtual ~application() = default;

  int32_t run();
};
}
