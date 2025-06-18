#pragma once

#include "common.hpp"

namespace geometry {
class size final {
public:
  size() noexcept;
  size(const size &) noexcept = default;
  size(float_t width, float_t height) noexcept;

  ~size() noexcept = default;

  void set_width(float_t width) noexcept;
  [[nodiscard]] float_t width() const noexcept;

  void set_height(float_t height) noexcept;
  [[nodiscard]] float_t height() const noexcept;

  bool operator==(const size &rhs) const noexcept;
  bool operator!=(const size &rhs) const noexcept;

  size operator*(float_t factor) const noexcept;
  size operator/(float_t factor) const noexcept;

  auto operator<=>(const size &) const noexcept = default;

  friend void from_json(const nlohmann::json &j, size &s);

private:
  float_t _width{0};
  float_t _height{0};
};
}
