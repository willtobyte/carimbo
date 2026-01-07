#pragma once

#include "common.hpp"

class io final {
public:
  io() = delete;
  ~io() = delete;

  [[nodiscard]] static bool exists(std::string_view filename) noexcept;

  [[nodiscard]] static std::vector<uint8_t> read(std::string_view filename);

  [[nodiscard]] static boost::container::small_vector<std::string, 32> enumerate(std::string_view directory);
};