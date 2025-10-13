#include "margin.hpp"

using namespace geometry;

float margin::top() const noexcept {
  return _top;
}

void margin::set_top(float value) noexcept {
  _top = value;
}

float margin::left() const noexcept {
  return _left;
}

void margin::set_left(float value) noexcept {
  _left = value;
}

float margin::bottom() const noexcept {
  return _bottom;
}

void margin::set_bottom(float value) noexcept {
  _bottom = value;
}

float margin::right() const noexcept {
  return _right;
}

void margin::set_right(float value) noexcept {
  _right = value;
}
