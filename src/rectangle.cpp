#include "rectangle.hpp"

using namespace geometry;

rectangle::rectangle(float_t x, float_t y, float_t w, float_t h)
  : _position{ x, y }, _size{ w, h } {}

rectangle::rectangle(const point &position, const class geometry::size &size)
  : _position(position), _size(size) {}

void rectangle::set_position(const point &position) {
  _position = position;
}

point rectangle::position() const {
  return _position;
}

void rectangle::set_size(const geometry::size &size) {
  _size = size;
}

size rectangle::size() const {
  return _size;
}

void rectangle::scale(float_t factor) {
  _size.set_width(_size.width() * factor);
  _size.set_height(_size.height() * factor);
}

bool rectangle::intersects(const rectangle &other) const {
  const auto [ax, ay] = std::pair{ _position.x(), _position.y() };
  const auto [aw, ah] = std::pair{ _size.width(), _size.height() };
  const auto [bx, by] = std::pair{ other._position.x(), other._position.y() };
  const auto [bw, bh] = std::pair{ other._size.width(), other._size.height() };

  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

bool rectangle::contains(const point &point) const {
  const auto [x, y] = std::pair{ point.x(), point.y() };
  const auto [left, top] = std::pair{ _position.x(), _position.y() };
  const auto [right, bottom] = std::pair{ left + _size.width(), top + _size.height() };

  return x >= left && x < right && y >= top && y < bottom;
}

bool rectangle::contains(float_t x, float_t y) const {
  const auto [left, top] = std::pair{ _position.x(), _position.y() };
  const auto [right, bottom] = std::pair{ left + _size.width(), top + _size.height() };

  return x >= left && x < right && y >= top && y < bottom;
}

rectangle::operator SDL_FRect() const {
  return SDL_FRect{
    .x = _position.x(),
    .y = _position.y(),
    .w = _size.width(),
    .h = _size.height()
  };
}

rectangle rectangle::operator+(const point &offset) const {
  return rectangle(_position + offset, _size);
}
