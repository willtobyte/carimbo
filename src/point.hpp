#pragma once

#include "common.hpp"

#include "size.hpp"

namespace geometry {
class point final {
public:
  point() = default;
  point(const point &) = default;
  point(float_t x, float_t y);

  ~point() = default;

  void set(float_t x, float_t y);

  float_t x() const;
  void set_x(float_t x);

  float_t y() const;
  void set_y(float_t y);

  operator SDL_FPoint() const;

  point operator+(const point &other) const;
  point &operator+=(const point &other);
  point &operator+=(std::pair<char, float_t> axis_offset);

  point operator-(const size &rhs) const;
  point operator-(const point &rhs) const;

  auto operator<=>(const point &) const = default;

  friend void from_json(const nlohmann::json &j, point &m);

private:
  float_t _x{0};
  float_t _y{0};
};

inline point operator*(const point &p, float_t factor) {
  return point(
      static_cast<float_t>(std::round(p.x() * factor)),
      static_cast<float_t>(std::round(p.y() * factor))
  );
}
}
