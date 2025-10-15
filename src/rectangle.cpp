#include "rectangle.hpp"

using namespace geometry;

rectangle::rectangle(float x, float y, float w, float h) noexcept
  : _position{ x, y }, _size{ w, h } {}

rectangle::rectangle(const point& position, const class geometry::size& size) noexcept
  : _position(position), _size(size) {}

void rectangle::set_position(float x, float y) noexcept {
  _position = {x, y};
}

void rectangle::set_position(const point& position) noexcept {
  _position = position;
}

point rectangle::position() const noexcept {
  return _position;
}

float rectangle::x() const noexcept {
  return _position.x();
}

float rectangle::y() const noexcept {
  return _position.y();
}

float rectangle::width() const noexcept {
  return _size.width();
}

float rectangle::height() const noexcept {
  return _size.height();
}

void rectangle::set_size(float width, float height) noexcept {
  _size = { width, height };
}

void rectangle::set_size(const geometry::size& size) noexcept {
  _size = size;
}

size rectangle::size() const noexcept {
  return _size;
}

void rectangle::scale(float factor) noexcept {
  _size.set_width(_size.width() * factor);
  _size.set_height(_size.height() * factor);
}

bool rectangle::intersects(const rectangle& other) const noexcept {
  const auto [ax, ay] = std::pair{ _position.x(), _position.y() };
  const auto [aw, ah] = std::pair{ _size.width(), _size.height() };
  const auto [bx, by] = std::pair{ other._position.x(), other._position.y() };
  const auto [bw, bh] = std::pair{ other._size.width(), other._size.height() };

  return ax < bx + bw && ax + aw > bx && ay < by + bh && ay + ah > by;
}

bool rectangle::contains(const point& point) const noexcept {
  const auto [x, y] = std::pair{ point.x(), point.y() };
  const auto [left, top] = std::pair{ _position.x(), _position.y() };
  const auto [right, bottom] = std::pair{ left + _size.width(), top + _size.height() };

  return x >= left && x < right && y >= top && y < bottom;
}

bool rectangle::contains(float x, float y) const noexcept {
  const auto [left, top] = std::pair{ _position.x(), _position.y() };
  const auto [right, bottom] = std::pair{ left + _size.width(), top + _size.height() };

  return x >= left && x < right && y >= top && y < bottom;
}

rectangle::operator SDL_FRect() const noexcept {
  return SDL_FRect{
    .x = _position.x(),
    .y = _position.y(),
    .w = _size.width(),
    .h = _size.height()
  };
}

rectangle rectangle::operator+(const point& offset) const noexcept {
  return rectangle(_position + offset, _size);
}

bool rectangle::operator==(const rectangle& other) const noexcept {
  constexpr float epsilon = std::numeric_limits<float>::epsilon();

  float ax = _position.x();
  float bx = other._position.x();
  float dx = std::fabs(ax - bx);
  float maxx = std::fabs(ax) > std::fabs(bx) ? std::fabs(ax) : std::fabs(bx);
  if (dx > epsilon * maxx && dx > epsilon) return false;

  float ay = _position.y();
  float by = other._position.y();
  float dy = std::fabs(ay - by);
  float maxy = std::fabs(ay) > std::fabs(by) ? std::fabs(ay) : std::fabs(by);
  if (dy > epsilon * maxy && dy > epsilon) return false;

  float aw = _size.width();
  float bw = other._size.width();
  float dw = std::fabs(aw - bw);
  float maxw = std::fabs(aw) > std::fabs(bw) ? std::fabs(aw) : std::fabs(bw);
  if (dw > epsilon * maxw && dw > epsilon) return false;

  float ah = _size.height();
  float bh = other._size.height();
  float dh = std::fabs(ah - bh);
  float maxh = std::fabs(ah) > std::fabs(bh) ? std::fabs(ah) : std::fabs(bh);
  if (dh > epsilon * maxh && dh > epsilon) return false;

  return true;
}

bool rectangle::operator!=(const rectangle& other) const noexcept {
  return !(*this == other);
}