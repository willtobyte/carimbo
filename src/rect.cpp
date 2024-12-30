#include "rect.hpp"
#include "point.hpp"
#include "size.hpp"

using namespace geometry;

rect::rect(const ::point &position, const ::size &size) noexcept
    : _position(position), _size(size) {}

void rect::set_position(const ::point &position) noexcept {
  _position = position;
}

::point rect::position() const noexcept {
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

rect::operator SDL_Rect() const noexcept {
  return SDL_Rect{
      .x = static_cast<int>(_position.x()),
      .y = static_cast<int>(_position.y()),
      .w = static_cast<int>(_size.width()),
      .h = static_cast<int>(_size.height())
  };
}

rect rect::operator+(const point &offset) const noexcept {
  return rect(_position + offset, _size);
}
