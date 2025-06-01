#pragma once

#include "common.hpp"

namespace steam {
  class achievement {
  public:
      achievement() = default;
      ~achievement() = delete;

      void unlock(std::string id);
  };
}
