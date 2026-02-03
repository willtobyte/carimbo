#pragma once

#include "common.hpp"

class user final {
public:
  user() noexcept = default;
  ~user() noexcept = default;

  [[nodiscard]] std::string persona() const noexcept;
  [[nodiscard]] std::vector<std::pair<uint64_t, std::string>> friends() const noexcept;
};
