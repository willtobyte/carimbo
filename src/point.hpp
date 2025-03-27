#pragma once

#include "common.hpp"

namespace geometry {
class point {
public:
  constexpr point() = default;
  constexpr point(const point &) noexcept = default;
  constexpr point(int32_t x, int32_t y) noexcept
      : _x(x), _y(y) {}

  ~point() = default;

  void set(int32_t x, int32_t y) noexcept;

  int32_t x() const noexcept;
  void set_x(int32_t x) noexcept;

  int32_t y() const noexcept;
  void set_y(int32_t y) noexcept;

  operator SDL_Point() const noexcept;

  point operator+(const point &other) const noexcept;
  point &operator+=(const point &other) noexcept;
  point &operator+=(std::pair<char, int32_t> axis_offset) noexcept;

  point operator-(const size &rhs) const noexcept;
  point operator-(const point &rhs) const noexcept;

  auto operator<=>(const point &) const = default;

  friend void from_json(const nlohmann::json &j, point &m) noexcept;

private:
  int32_t _x{0};
  int32_t _y{0};
};

inline point operator*(const point &p, float_t factor) noexcept {
  return point(
      static_cast<int32_t>(std::round(p.x() * factor)),
      static_cast<int32_t>(std::round(p.y() * factor))
  );
}
}
