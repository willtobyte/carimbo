#include "margin.hpp"

using namespace geometry;

int32_t margin::top() const noexcept {
  return _top;
}

void margin::set_top(int32_t value) noexcept {
  _top = value;
}

int32_t margin::left() const noexcept {
  return _left;
}

void margin::set_left(int32_t value) noexcept {
  _left = value;
}

int32_t margin::bottom() const noexcept {
  return _bottom;
}

void margin::set_bottom(int32_t value) noexcept {
  _bottom = value;
}

int32_t margin::right() const noexcept {
  return _right;
}

void margin::set_right(int32_t value) noexcept {
  _right = value;
}
