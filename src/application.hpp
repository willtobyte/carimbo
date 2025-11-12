#pragma once

#include "common.hpp"

#include "noncopyable.hpp"

namespace framework {
class application final : private noncopyable {
public:
  application(int argc, char** argv);
  ~application() = default;

  [[nodiscard]] int32_t run();
};
}