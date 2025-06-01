#pragma once

#include "common.hpp"

namespace steam {
  class achievement {
  public:
      achievement() = default;
      ~achievement() = default;

      void unlock(std::string id);
  };
}
