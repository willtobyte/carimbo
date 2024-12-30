#pragma once

#include "common.hpp"

namespace geometry {
class point {
public:
  point() = default;
  point(int32_t x, int32_t y) noexcept;
  point(const point &other) noexcept;
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

  friend void from_json(const nlohmann::json &j, point &m) noexcept;

private:
  int32_t _x{0};
  int32_t _y{0};
};
}
