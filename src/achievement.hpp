#pragma once

#include "common.hpp"

namespace steam {
  class achievement final {
  public:
      achievement() noexcept = default;
      ~achievement() noexcept = default;

      void unlock(std::string id) noexcept;
  };
}
