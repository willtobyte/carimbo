#pragma once

#include "common.hpp"

class desktop final {
public:
  constexpr desktop() noexcept = default;
  constexpr ~desktop() noexcept = default;

  [[nodiscard]] std::optional<std::string_view> folder() const noexcept;
};

class operatingsystem final {
public:
  constexpr operatingsystem() noexcept = default;
  constexpr ~operatingsystem() noexcept = default;

  [[nodiscard]] int compute() const noexcept;

  [[nodiscard]] int memory() const noexcept;

  [[nodiscard]] std::string_view name() const noexcept;
};
