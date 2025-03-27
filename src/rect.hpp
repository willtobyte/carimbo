#pragma once

#include "common.hpp"
#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rect {
public:
  rect() noexcept = default;
  rect(const rect &other) noexcept = default;
  rect(const geometry::point &position, const geometry::size &size) noexcept;

  ~rect() noexcept = default;

  void set_position(const geometry::point &position) noexcept;
  geometry::point position() const noexcept;

  void set_size(const geometry::size &size) noexcept;
  geometry::size size() const noexcept;

  void scale(float_t factor) noexcept;

  bool intersects(const rect &other) const noexcept;

  bool contains(const geometry::point &p) const noexcept;

  operator SDL_Rect() const noexcept;

  rect operator+(const geometry::point &offset) const noexcept;

  auto operator<=>(const rect &) const = default;

  friend void from_json(const nlohmann::json &j, rect &r) noexcept;

private:
  point _position;
  geometry::size _size;
};
}
