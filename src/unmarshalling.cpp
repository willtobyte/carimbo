#include "common.hpp"

void from_json(const nlohmann::json& j, keyframe& o) {
  o.frame = j.at("rectangle").get<vec4>();

  if (j.contains("offset")) {
    o.offset = j.at("offset").get<vec2>();
  }

  o.duration = j.at("duration").get<uint64_t>();
}

void from_json(const nlohmann::json& j, bounds& o) {
  if (j.contains("type")) {
    o.type = j.at("type").get<uint16_t>();
  }

  if (j.contains("reagents")) {
    for (const auto& r : j.at("reagents")) {
      o.reagents.set(r.get<uint8_t>());
    }
  }

  o.rectangle = j.at("rectangle").get<vec4>();
}