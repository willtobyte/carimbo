#pragma once

#include "common.hpp"
#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rect {
public:
  constexpr rect() noexcept = default;
  constexpr rect(const rect &other) noexcept = default;
  constexpr rect(const class point &position, const class size &size) noexcept
      : _position(position), _size(size) {}

  ~rect() noexcept = default;

  void set_position(const class point &position) noexcept;
  point position() const noexcept;

  void set_size(const class size &size) noexcept;
  size size() const noexcept;

  void scale(float_t factor) noexcept;

  bool intersects(const rect &other) const noexcept;

  bool contains(const point &p) const noexcept;

  operator SDL_Rect() const noexcept;

  rect operator+(const point &offset) const noexcept;

  auto operator<=>(const rect &) const = default;

  friend void from_json(const nlohmann::json &j, rect &r) noexcept;

private:
  point _position;
  geometry::size _size;
};
}
