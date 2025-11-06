#pragma once

#include "common.hpp"

namespace steam {
  class achievement final {
  public:
      achievement() = default;
      ~achievement() = default;

      void unlock(const std::string& id);
  };
}
