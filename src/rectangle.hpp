#pragma once

#include "common.hpp"

#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rectangle final {
public:
  rectangle() noexcept = default;
  rectangle(const rectangle &other) noexcept = default;
  rectangle(float_t x, float_t y, float_t width, float_t height) noexcept;
  rectangle(const geometry::point &position, const geometry::size &size) noexcept;

  ~rectangle() noexcept = default;

  float_t x() const noexcept;
  float_t y() const noexcept;
  float_t width() const noexcept;
  float_t height() const noexcept;

  void set_position(const geometry::point &position) noexcept;
  geometry::point position() const noexcept;

  void set_size(const geometry::size &size) noexcept;
  geometry::size size() const noexcept;

  void scale(float_t factor) noexcept;

  bool intersects(const rectangle &other) const noexcept;

  bool contains(const geometry::point &point) const noexcept;
  bool contains(float_t x, float_t y) const noexcept;

  operator SDL_FRect() const noexcept;

  rectangle operator+(const geometry::point &offset) const noexcept;

  auto operator<=>(const rectangle &) const noexcept = default;

  friend void from_json(const nlohmann::json &j, rectangle &r);

private:
  point _position{};
  geometry::size _size{};
};
}
