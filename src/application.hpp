#pragma once

#include "common.hpp"

namespace framework {
class application final : private noncopyable {
public:
  application(int argc, char **argv) noexcept;
  virtual ~application() noexcept = default;

  int32_t run();
};
}
