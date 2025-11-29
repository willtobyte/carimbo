#pragma once

#include "common.hpp"

#include "geometry.hpp"

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

  friend void from_json(const nlohmann::json& j, offset& o);
};

struct frame final {
  int duration;
  offset offset;
  quad quad;

  friend void from_json(const nlohmann::json& j, frame& o);
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

struct timeline final {
  std::string next;
  std::optional<b2AABB> box;
  std::vector<frame> frames;
  std::vector<uint16_t> durations;
  uint16_t current{0};
  uint64_t tick{0};

   friend void from_json(const nlohmann::json& j, timeline& o);
};

struct animator final {
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

struct state final {
  bool dirty;
  bool redraw;
  uint16_t current_frame{0};
  uint64_t tick{0};
  std::optional<std::string> action;
};

struct renderable final {
  int z;
  bool visible{true};
};

struct metadata final {
  std::string kind;
};

struct callbacks {
  std::function<void(std::shared_ptr<entityproxy>)> on_hover;
  std::function<void(std::shared_ptr<entityproxy>)> on_unhover;
  std::function<void(std::shared_ptr<entityproxy>, float, float)> on_touch;

  std::shared_ptr<entityproxy> self;
};
