#pragma once

#include "common.hpp"
#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rectangle {
public:
  rectangle() noexcept = default;
  rectangle(const rectangle &other) noexcept = default;
  rectangle(const geometry::point &position, const geometry::size &size) noexcept;

  ~rectangle() noexcept = default;

  void set_position(const geometry::point &position) noexcept;
  geometry::point position() const noexcept;

  void set_size(const geometry::size &size) noexcept;
  geometry::size size() const noexcept;

  void scale(float_t factor) noexcept;

  bool intersects(const rectangle &other) const noexcept;

  bool contains(const geometry::point &point) const noexcept;

  operator SDL_rectangle() const noexcept;

  rectangle operator+(const geometry::point &offset) const noexcept;

  auto operator<=>(const rectangle &) const = default;

  friend void from_json(const nlohmann::json &j, rectangle &r) noexcept;

private:
  point _position{};
  geometry::size _size{};
};
}
