#include "rect.hpp"

using namespace geometry;

rect::rect(const geometry::point &position, const geometry::size &size) noexcept
    : _position(position), _size(size) {}

void rect::set_position(const geometry::point &position) noexcept {
  _position = position;
}

geometry::point rect::position() const noexcept {
  return _position;
}

void rect::set_size(const geometry::size &size) noexcept {
  _size = size;
}

geometry::size rect::size() const noexcept {
  return _size;
}

void rect::scale(float_t factor) noexcept {
  _size.set_width(static_cast<int>(_size.width() * factor));
  _size.set_height(static_cast<int>(_size.height() * factor));
}

bool rect::intersects(const rect &other) const noexcept {
  const auto ax1 = _position.x(), ay1 = _position.y();
  const auto ax2 = ax1 + _size.width(), ay2 = ay1 + _size.height();
  const auto bx1 = other._position.x(), by1 = other._position.y();
  const auto bx2 = bx1 + other._size.width(), by2 = by1 + other._size.height();

  return ax1 < bx2 && ax2 > bx1 && ay1 < by2 && ay2 > by1;
}

bool rect::contains(const geometry::point &point) const noexcept {
  const auto x0 = _position.x(), y0 = _position.y();
  const auto x1 = x0 + _size.width(), y1 = y0 + _size.height();
  return point.x() >= x0 && point.x() < x1 && point.y() >= y0 && point.y() < y1;
}

rect::operator SDL_Rect() const noexcept {
  return SDL_Rect{
      .x = static_cast<int>(_position.x()),
      .y = static_cast<int>(_position.y()),
      .w = static_cast<int>(_size.width()),
      .h = static_cast<int>(_size.height())
  };
}

rect rect::operator+(const geometry::point &offset) const noexcept {
  return rect(_position + offset, _size);
}
