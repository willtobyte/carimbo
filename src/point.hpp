#pragma once

#include "common.hpp"

#include "size.hpp"

namespace geometry {
class point final {
public:
  point() = default;
  point(const point&) = default;
  point(float x, float y);

  ~point() = default;

  void set(float x, float y);

  [[nodiscard]] float x() const;
  void set_x(float x);

  [[nodiscard]] float y() const;
  void set_y(float y);

  operator SDL_FPoint() const;

  [[nodiscard]] point operator+(const point& other) const;
  point& operator+=(const point& other);
  point& operator+=(const std::pair<char, float>& offset);

  [[nodiscard]] point operator-(const size& rhs) const;
  [[nodiscard]] point operator-(const point& rhs) const;

  [[nodiscard]] bool operator==(const point& other) const;

  friend void from_json(const nlohmann::json& j, point& m);

private:
  float _x{0};
  float _y{0};
};

[[nodiscard]] inline point operator*(const point& p, float factor) {
  return point(
      std::round(p.x() * factor),
      std::round(p.y() * factor)
  );
}
}