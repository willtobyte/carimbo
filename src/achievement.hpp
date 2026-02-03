#pragma once

#include "common.hpp"

class achievement final {
public:
  achievement() noexcept = default;
  ~achievement() noexcept = default;

  void unlock(std::string_view id) noexcept;
};
