#pragma once

#include "common.hpp"

namespace platform {
class desktop {
  public:
    desktop() noexcept = default;
    ~desktop() noexcept = default;

    std::optional<std::string> folder() const noexcept;
};

class operatingsystem {
  public:
    operatingsystem() noexcept = default;
    ~operatingsystem() noexcept = default;

    std::string name() const noexcept;
};
}
