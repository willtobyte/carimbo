#include "rectangle.hpp"

using namespace geometry;

rectangle::rectangle(float_t x, float_t y, float_t w, float_t h)
  : _position{ x, y }, _size{ w, h } {}

rectangle::rectangle(const geometry::point &position, const geometry::size &size)
    : _position(position), _size(size) {}

void rectangle::set_position(const geometry::point &position) {
  _position = position;
}

geometry::point rectangle::position() const {
  return _position;
}

void rectangle::set_size(const geometry::size &size) {
  _size = size;
}

geometry::size rectangle::size() const {
  return _size;
}

void rectangle::scale(float_t factor) {
  _size.set_width(static_cast<int>(_size.width() * factor));
  _size.set_height(static_cast<int>(_size.height() * factor));
}

bool rectangle::intersects(const rectangle &other) const {
  const auto ax1 = _position.x(), ay1 = _position.y();
  const auto ax2 = ax1 + _size.width(), ay2 = ay1 + _size.height();
  const auto bx1 = other._position.x(), by1 = other._position.y();
  const auto bx2 = bx1 + other._size.width(), by2 = by1 + other._size.height();

  return ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1;
}

bool rectangle::contains(const geometry::point &point) const {
  const auto x0 = _position.x(), y0 = _position.y();
  const auto x1 = x0 + _size.width(), y1 = y0 + _size.height();
  return point.x() >= x0 && point.x() < x1 && point.y() >= y0 && point.y() < y1;
}

rectangle::operator SDL_FRect() const {
  return SDL_FRect{
      .x = _position.x(),
      .y = _position.y(),
      .w = _size.width(),
      .h = _size.height()
  };
}

rectangle rectangle::operator+(const geometry::point &offset) const {
  return rectangle(_position + offset, _size);
}
