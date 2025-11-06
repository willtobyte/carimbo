#include "margin.hpp"

using namespace geometry;

float margin::top() const {
  return _top;
}

void margin::set_top(float value) {
  _top = value;
}

float margin::left() const {
  return _left;
}

void margin::set_left(float value) {
  _left = value;
}

float margin::bottom() const {
  return _bottom;
}

void margin::set_bottom(float value) {
  _bottom = value;
}

float margin::right() const {
  return _right;
}

void margin::set_right(float value) {
  _right = value;
}
