#pragma once

#include "common.hpp"

namespace geometry {
class size final {
public:
  size() noexcept;
  size(const size&) noexcept = default;
  size(float width, float height) noexcept;

  ~size() noexcept = default;

  void set_width(float width) noexcept;
  [[nodiscard]] float width() const noexcept;

  void set_height(float height) noexcept;
  [[nodiscard]] float height() const noexcept;

  bool operator==(const size& rhs) const noexcept;
  bool operator!=(const size& rhs) const noexcept;

  size operator*(float factor) const noexcept;
  size operator/(float factor) const noexcept;

  auto operator<=>(const size&) const noexcept = default;

  friend void from_json(const nlohmann::json& j, size& s);

private:
  float _width{0};
  float _height{0};
};
}
