#pragma once

#include "common.hpp"

#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rectangle final {
public:
  rectangle() noexcept = default;
  rectangle(const rectangle& other) noexcept = default;
  rectangle(float x, float y, float width, float height) noexcept;
  rectangle(const geometry::point& position, const geometry::size& size) noexcept;

  ~rectangle() noexcept = default;

  float x() const noexcept;
  float y() const noexcept;
  float width() const noexcept;
  float height() const noexcept;

  void set_position(float x, float y) noexcept;
  void set_position(const geometry::point& position) noexcept;
  geometry::point position() const noexcept;

  void set_size(float width, float height) noexcept;
  void set_size(const geometry::size& size) noexcept;
  geometry::size size() const noexcept;

  void scale(float factor) noexcept;

  bool intersects(const rectangle& other) const noexcept;

  bool contains(const geometry::point& point) const noexcept;
  bool contains(float x, float y) const noexcept;

  operator SDL_FRect() const noexcept;

  rectangle operator+(const geometry::point& offset) const noexcept;

  auto operator<=>(const rectangle&) const noexcept = default;

  friend void from_json(const nlohmann::json& j, rectangle& r);

private:
  point _position{};
  geometry::size _size{};
};
}
