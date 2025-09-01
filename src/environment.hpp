#pragma once

#include "common.hpp"

namespace platform {
class desktop {
  public:
    desktop() noexcept = default;
    ~desktop() noexcept = default;

    std::optional<std::string> path() const noexcept;
};
}
