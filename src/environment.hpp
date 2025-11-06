#pragma once

#include "common.hpp"

namespace platform {
class desktop {
  public:
    desktop() = default;
    ~desktop() = default;

    std::optional<std::string> folder() const;
};

class operatingsystem {
  public:
    operatingsystem() = default;
    ~operatingsystem() = default;

    int32_t compute() const;

    int32_t memory() const;

    std::string name() const;
};
}
