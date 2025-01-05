#pragma once

#include "common.hpp"
#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rect {
public:
  rect() noexcept = default;
  rect(const point &position, const size &size) noexcept;
  rect(const rect &other) noexcept = default;
  ~rect() noexcept = default;

  void set_position(const point &position) noexcept;
  point position() const noexcept;

  void set_size(const geometry::size &size) noexcept;
  geometry::size size() const noexcept;

  void scale(float_t factor) noexcept;

  bool intersects(const rect &other) const noexcept;

  operator SDL_Rect() const noexcept;

  rect operator+(const point &offset) const noexcept;

  friend void from_json(const nlohmann::json &j, rect &r) noexcept;

private:
  point _position;
  geometry::size _size;
};
}
