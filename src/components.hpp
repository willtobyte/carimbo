#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

struct transform final {
	vec2 position;
	double angle;
	float scale;
};

struct tint final {
  uint8_t r{0};
  uint8_t g{0};
  uint8_t b{0};
  uint8_t a{255};
};

struct offset {
  float x{.0f};
  float y{.0f};

  friend void from_json(const nlohmann::json& j, offset& o) {
    j["x"].get_to(o.x);
    j["y"].get_to(o.y);
  }
};

struct frame final {
  int duration;
  offset offset;
  quad quad;

  friend void from_json(const nlohmann::json& j, frame& o) {
    j["duration"].get_to(o.duration);
    if (j.contains("offset")) {
      j["offset"].get_to(o.offset);
    }
    j["quad"].get_to(o.quad);
  }
};

enum class bodytype : uint8_t {
  static_body = 0,
  kinematic,
  dynamic
};

struct physics final {
  b2BodyId body;
  b2ShapeId shape;
  bodytype type{bodytype::static_body};
  bool dirty{true};
  bool enabled{true};

  bool is_valid() const noexcept {
    return b2Body_IsValid(body);
  }
};

inline void from_json(const nlohmann::json& j, b2AABB& o) {
  const auto x = j["x"].get<float>();
  const auto y = j["y"].get<float>();
  const auto w = j["w"].get<float>();
  const auto h = j["h"].get<float>();

  o.lowerBound = b2Vec2(x - epsilon, y - epsilon);
  o.upperBound = b2Vec2(x + w + epsilon, y + h + epsilon);
}

struct timeline final {
  bool oneshot{false};
  std::string next;
  std::optional<b2AABB> hitbox;
  std::vector<frame> frames;
  std::vector<uint16_t> durations;
  uint16_t current{0};
  uint64_t tick{0};

  friend void from_json(const nlohmann::json& j, timeline& o) {
    if (j.contains("oneshot")) {
      j["oneshot"].get_to(o.oneshot);
    }

    if (j.contains("next")) {
      j["next"].get_to(o.next);
    }

    if (j.contains("hitbox")) {
      b2AABB b;
      j["hitbox"]["quad"].get_to(b);
      o.hitbox = b;
    }

    if (j.contains("frames")) {
      j["frames"].get_to(o.frames);
    }
  }
};

struct atlas final {
  std::unordered_map<std::string, timeline> timelines;

  const timeline& operator[](const std::string& action) const {
    auto it = timelines.find(action);
    assert(it != timelines.end() && "timeline not found");
    return it->second;
  }
};

struct sprite final {
  std::shared_ptr<pixmap> pixmap;
};

struct playback final {
  bool dirty;
  bool redraw;
  uint16_t current_frame{0};
  uint64_t tick{0};
  std::optional<std::string> action;
  const timeline* cache{nullptr};
};

struct renderable final {
  int z;
  bool visible{true};
};

struct metadata final {
  std::string kind;
};

struct orientation final {
  flip flip{flip::none};
};

struct callbacks {
  std::function<void(std::shared_ptr<entityproxy>, std::string_view)> on_mail;

  std::function<void(std::shared_ptr<entityproxy>)> on_hover;
  std::function<void(std::shared_ptr<entityproxy>)> on_unhover;
  std::function<void(std::shared_ptr<entityproxy>, float, float)> on_touch;

  std::shared_ptr<entityproxy> self;
};
