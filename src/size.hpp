#pragma once

#include "common.hpp"

namespace geometry {
class size {
public:
  constexpr size() noexcept = default;
  constexpr size(const size &) noexcept = default;
  constexpr size(int32_t width, int32_t height) noexcept
      : _width(width), _height(height) {}

  ~size() = default;

  void set_width(int32_t width) noexcept;
  int32_t width() const noexcept;
  void set_height(int32_t height) noexcept;
  int32_t height() const noexcept;

  bool operator==(const size &rhs) const noexcept;
  bool operator!=(const size &rhs) const noexcept;

  size operator*(float_t factor) const noexcept;

  auto operator<=>(const size &) const = default;

  friend void from_json(const nlohmann::json &j, size &s) noexcept;

private:
  int32_t _width{0};
  int32_t _height{0};
};
}
