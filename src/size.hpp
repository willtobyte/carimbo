#pragma once

#include "common.hpp"

namespace geometry {
class size final {
public:
  size();
  size(const size &) = default;
  size(float_t width, float_t height);

  ~size() = default;

  void set_width(float_t width);
  [[nodiscard]] float_t width() const;

  void set_height(float_t height);
  [[nodiscard]] float_t height() const;

  bool operator==(const size &rhs) const;
  bool operator!=(const size &rhs) const;

  size operator*(float_t factor) const;
  size operator/(float_t factor) const;

  auto operator<=>(const size &) const = default;

  friend void from_json(const nlohmann::json &j, size &s);

private:
  float_t _width{0};
  float_t _height{0};
};
}
