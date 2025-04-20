#include "margin.hpp"

using namespace geometry;

int32_t margin::top() const {
  return _top;
}

void margin::set_top(int32_t value) {
  _top = value;
}

int32_t margin::left() const {
  return _left;
}

void margin::set_left(int32_t value) {
  _left = value;
}

int32_t margin::bottom() const {
  return _bottom;
}

void margin::set_bottom(int32_t value) {
  _bottom = value;
}

int32_t margin::right() const {
  return _right;
}

void margin::set_right(int32_t value) {
  _right = value;
}
