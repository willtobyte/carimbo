#pragma once

#include "common.hpp"

#include "size.hpp"

namespace geometry {
class point {
public:
  point() = default;
  point(const point &) noexcept = default;
  point(float_t x, float_t y) noexcept;

  ~point() = default;

  void set(float_t x, float_t y) noexcept;

  float_t x() const noexcept;
  void set_x(float_t x) noexcept;

  float_t y() const noexcept;
  void set_y(float_t y) noexcept;

  operator SDL_FPoint() const noexcept;

  point operator+(const point &other) const noexcept;
  point &operator+=(const point &other) noexcept;
  point &operator+=(std::pair<char, float_t> axis_offset) noexcept;

  point operator-(const size &rhs) const noexcept;
  point operator-(const point &rhs) const noexcept;

  auto operator<=>(const point &) const = default;

  friend void from_json(const nlohmann::json &j, point &m) noexcept;

private:
  float_t _x{0};
  float_t _y{0};
};

inline point operator*(const point &p, float_t factor) noexcept {
  return point(
      static_cast<float_t>(std::round(p.x() * factor)),
      static_cast<float_t>(std::round(p.y() * factor))
  );
}
}
