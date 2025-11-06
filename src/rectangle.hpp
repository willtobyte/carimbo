#pragma once

#include "common.hpp"

#include "point.hpp"
#include "size.hpp"

namespace geometry {
class rectangle final {
public:
  rectangle() = default;
  rectangle(const rectangle& other) = default;
  rectangle(float x, float y, float width, float height);
  rectangle(const geometry::point& position, const geometry::size& size);

  ~rectangle() = default;

  float x() const;
  float y() const;
  float width() const;
  float height() const;

  void set_position(float x, float y);
  void set_position(const geometry::point& position);
  geometry::point position() const;

  void set_size(float width, float height);
  void set_size(const geometry::size& size);
  geometry::size size() const;

  void scale(float factor);

  bool intersects(const rectangle& other) const;

  bool contains(const geometry::point& point) const;
  bool contains(float x, float y) const;

  operator SDL_FRect() const;

  rectangle operator+(const geometry::point& offset) const;
  bool operator==(const rectangle& other) const;
  bool operator!=(const rectangle& other) const;

  friend void from_json(const nlohmann::json& j, rectangle& r);

private:
  point _position{};
  geometry::size _size{};
};
}
