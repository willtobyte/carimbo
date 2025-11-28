#include "common.hpp"

#include "cursor.hpp"
#include "geometry.hpp"

void from_json(const nlohmann::json& j, keyframe& o) {
  o.frame = j["quad"].get<quad>();

  if (j.contains("offset")) {
    o.offset = j["offset"].get<vec2>();
  }

  o.duration = j["duration"].get<uint64_t>();
}

void from_json(const nlohmann::json& j, bounds& o) {
  if (j.contains("type")) {
    o.type = j["type"].get<uint16_t>();
  }

  // if (j.contains("reagents")) {
  //   for (const auto& r : j["reagents")) {
  //     o.reagents.set(r.get<uint8_t>());
  //   }
  // }

  o.rectangle = j["quad"].get<quad>();
}

void from_json(const nlohmann::json& j, vec2& o) {
  j["x"].get_to(o.x);
  j["y"].get_to(o.y);
}

void from_json(const nlohmann::json& j, vec3& o) {
  j["x"].get_to(o.x);
  j["y"].get_to(o.y);
  j["z"].get_to(o.z);
}

void from_json(const nlohmann::json& j, quad& o) {
  j["x"].get_to(o.x);
  j["y"].get_to(o.y);
  j["w"].get_to(o.w);
  j["h"].get_to(o.h);
}

void from_json(const nlohmann::json& j, offset& o) {
  j["x"].get_to(o.x);
  j["y"].get_to(o.y);
}

void from_json(const nlohmann::json& j, frame& o) {
  j["duration"].get_to(o.duration);
  if (j.contains("offset")) {
    j["offset"].get_to(o.offset);
  }
  j["quad"].get_to(o.quad);
}

void from_json(const nlohmann::json& j, b2AABB& o) {
  const auto x = j["x"].get<float>();
  const auto y = j["y"].get<float>();
  const auto w = j["w"].get<float>();
  const auto h = j["h"].get<float>();

  o.lowerBound = b2Vec2(x - epsilon, y - epsilon);
  o.upperBound = b2Vec2(x + w + epsilon, y + h + epsilon);
}

void from_json(const nlohmann::json& j, timeline& o) {
  if (j.contains("next")) {
    j["next"].get_to(o.next);
  }

  if (j.contains("hitbox")) {
    b2AABB box;
    j["hitbox"]["quad"].get_to(box);
    o.box = box;
  }

  if (j.contains("frames")) {
    j["frames"].get_to(o.frames);
  }
}
