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

    int32_t compute() const noexcept;

    int32_t memory() const noexcept;

    std::string name() const noexcept;
};
}
