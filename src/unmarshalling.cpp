#include "common.hpp"

namespace geometry {
void from_json(const nlohmann::json& j, size& s) {
  j.at("width").get_to(s._width);
  j.at("height").get_to(s._height);
}

void from_json(const nlohmann::json& j, margin& m) {
  j.at("top").get_to(m._top);
  j.at("left").get_to(m._left);
  j.at("bottom").get_to(m._bottom);
  j.at("right").get_to(m._right);
}

void from_json(const nlohmann::json& j, point& m) {
  j.at("x").get_to(m._x);
  j.at("y").get_to(m._y);
}

void from_json(const nlohmann::json& j, rectangle& r) {
  r._position = point{j.at("x").get<float_t>(), j.at("y").get<float_t>()};
  r._size = geometry::size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};
}
}
