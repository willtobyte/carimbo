#pragma once

#include "common.hpp"

namespace geometry {
class size final {
public:
  size();
  size(const size&) = default;
  size(float width, float height);

  ~size() = default;

  void set_width(float width);
  [[nodiscard]] float width() const;

  void set_height(float height);
  [[nodiscard]] float height() const;

  bool operator==(const size& rhs) const;
  bool operator!=(const size& rhs) const;

  size operator*(float factor) const;
  size operator/(float factor) const;

  friend void from_json(const nlohmann::json& j, size& s);

private:
  float _width{0};
  float _height{0};
};
}
