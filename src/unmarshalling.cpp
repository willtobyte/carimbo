#include "common.hpp"

namespace geometry {
void from_json(const nlohmann::json &j, size &s) noexcept {
  j.at("width").get_to(s._width);
  j.at("height").get_to(s._height);
}

void from_json(const nlohmann::json &j, margin &m) {
  j.at("top").get_to(m._top);
  j.at("left").get_to(m._left);
  j.at("bottom").get_to(m._bottom);
  j.at("right").get_to(m._right);
}

void from_json(const nlohmann::json &j, point &m) noexcept {
  j.at("x").get_to(m._x);
  j.at("y").get_to(m._y);
}

void from_json(const nlohmann::json &j, rect &r) noexcept {
  r._position = point{j.at("x").get<int>(), j.at("y").get<int>()};
  r._size = geometry::size{j.at("width").get<int>(), j.at("height").get<int>()};
}
}
