#pragma once

#include "common.hpp"

namespace framework {
template <typename T>
class singleton {
public:
  virtual ~singleton() noexcept = default;

  static std::shared_ptr<T> instance();
};
}
