#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

using action_id = entt::id_type;
inline constexpr action_id no_action = 0;

[[nodiscard]] inline std::unordered_map<action_id, std::string>& action_registry() noexcept {
  static std::unordered_map<action_id, std::string> registry;
  return registry;
}

[[nodiscard]] inline action_id make_action(std::string_view action) noexcept {
  if (action.empty()) return no_action;
  const auto id = entt::hashed_string{action.data(), action.size()}.value();
  action_registry().try_emplace(id, action);
  return id;
}

[[nodiscard]] inline std::optional<std::string_view> action_name(action_id id) noexcept {
  if (id == no_action) return std::nullopt;
  auto& reg = action_registry();
  auto it = reg.find(id);
  return it != reg.end() ? std::optional<std::string_view>{it->second} : std::nullopt;
}

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
  action_id next{no_action};
  std::optional<b2AABB> hitbox;
  std::vector<frame> frames;

  friend void from_json(const nlohmann::json& j, timeline& o) {
    if (j.contains("oneshot")) {
      j["oneshot"].get_to(o.oneshot);
    }

    if (j.contains("next")) {
      o.next = make_action(j["next"].get<std::string_view>());
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
  entt::dense_map<action_id, timeline> timelines;

  const timeline* find(action_id id) const noexcept {
    auto it = timelines.find(id);
    return it != timelines.end() ? &it->second : nullptr;
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
  action_id action{no_action};
  const timeline* timeline{nullptr};
};

struct renderable final {
  int z;
  bool visible{true};
};

struct metadata final {
  action_id kind{no_action};
};

struct orientation final {
  flip flip{flip::none};
};

struct callbacks {
  functor on_mail;
  functor on_hover;
  functor on_unhover;
  functor on_touch;

  std::shared_ptr<entityproxy> self;
};
