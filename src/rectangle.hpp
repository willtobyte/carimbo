#pragma once

#include "common.hpp"

#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rectangle final {
public:
  rectangle() = default;
  rectangle(const rectangle &other) = default;
  rectangle(float_t x, float_t y, float_t width, float_t height);
  rectangle(const geometry::point &position, const geometry::size &size);

  ~rectangle() = default;

  float_t x() const noexcept;
  float_t y() const noexcept;
  float_t width() const noexcept;
  float_t height() const noexcept;

  void set_position(const geometry::point &position);
  geometry::point position() const;

  void set_size(const geometry::size &size);
  geometry::size size() const;

  void scale(float_t factor);

  bool intersects(const rectangle &other) const;

  bool contains(const geometry::point &point) const;
  bool contains(float_t x, float_t y) const;

  operator SDL_FRect() const;

  rectangle operator+(const geometry::point &offset) const;

  auto operator<=>(const rectangle &) const = default;

  friend void from_json(const nlohmann::json &j, rectangle &r);

private:
  point _position{};
  geometry::size _size{};
};
}
