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

  float x() const;
  void set_x(float x);

  float y() const;
  void set_y(float y);

  operator SDL_FPoint() const;

  point operator+(const point& other) const;
  point& operator+=(const point& other);
  point& operator+=(const std::pair<char, float>& offset);

  point operator-(const size& rhs) const;
  point operator-(const point& rhs) const;

  bool operator==(const point& other) const;
  bool operator!=(const point& other) const;

  friend void from_json(const nlohmann::json& j, point& m);

private:
  float _x{0};
  float _y{0};
};

inline point operator*(const point& p, float factor) {
  return point(
      static_cast<float>(std::round(p.x() * factor)),
      static_cast<float>(std::round(p.y() * factor))
  );
}
}
