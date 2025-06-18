#pragma once

#include "noncopyable.hpp"

namespace framework {
class application final : private noncopyable {
public:
  application(int argc, char **argv);
  virtual ~application() noexcept = default;

  int32_t run();
};
}
