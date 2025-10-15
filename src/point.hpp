#pragma once

#include "common.hpp"

#include "size.hpp"

namespace geometry {
class point final {
public:
  point() noexcept = default;
  point(const point&) noexcept = default;
  point(float x, float y) noexcept;

  ~point() noexcept = default;

  void set(float x, float y) noexcept;

  float x() const noexcept;
  void set_x(float x) noexcept;

  float y() const noexcept;
  void set_y(float y) noexcept;

  operator SDL_FPoint() const noexcept;

  point operator+(const point& other) const noexcept;
  point& operator+=(const point& other) noexcept;
  point& operator+=(const std::pair<char, float>& offset) noexcept;

  point operator-(const size& rhs) const noexcept;
  point operator-(const point& rhs) const noexcept;

  bool operator==(const point& other) const noexcept;

  friend void from_json(const nlohmann::json& j, point& m);

private:
  float _x{0};
  float _y{0};
};

inline point operator*(const point& p, float factor) noexcept {
  return point(
      static_cast<float>(std::round(p.x() * factor)),
      static_cast<float>(std::round(p.y() * factor))
  );
}
}
