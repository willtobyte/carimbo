#include "common.hpp"

namespace geometry {
void from_json(const nlohmann::json& j, size& o) {
  j.at("width").get_to(o._width);
  j.at("height").get_to(o._height);
}

void from_json(const nlohmann::json& j, margin& o) {
  j.at("top").get_to(o._top);
  j.at("left").get_to(o._left);
  j.at("bottom").get_to(o._bottom);
  j.at("right").get_to(o._right);
}

void from_json(const nlohmann::json& j, point& o) {
  j.at("x").get_to(o._x);
  j.at("y").get_to(o._y);
}

void from_json(const nlohmann::json& j, rectangle& o) {
  o._position = point{j.at("x").get<float_t>(), j.at("y").get<float_t>()};
  o._size = size{j.at("width").get<float_t>(), j.at("height").get<float_t>()};
}
}

namespace framework {
void from_json(const nlohmann::json& j, keyframe& o) {
  o.frame = j.at("rectangle").get<geometry::rectangle>();

  if (j.contains("offset")) {
    o.offset = j.at("offset").get<geometry::point>();
  }

  o.duration = j.at("duration").get<uint64_t>();
}

void from_json(const nlohmann::json& j, framework::hitbox& o) {
  if (j.contains("type")) {
    o.type = j.at("type").get<uint16_t>();
  }

  if (j.contains("reagents")) {
    for (const auto& r : j.at("reagents")) {
      o.reagents.set(r.get<uint8_t>());
    }
  }

  o.rectangle = j.at("rectangle").get<geometry::rectangle>();
}
}
