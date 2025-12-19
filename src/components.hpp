#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

using symbol = entt::id_type;
inline constexpr symbol no_action = 0;

static boost::unordered_flat_map<symbol, std::string> _registry;

[[nodiscard]] inline symbol _resolve(std::string_view action) noexcept {
  if (action.empty()) [[unlikely]] {
    return no_action;
  }

  const auto id = entt::hashed_string{action.data(), action.size()}.value();
  _registry.try_emplace(id, action);
  return id;
}

[[nodiscard]] inline std::optional<std::string_view> action_name(symbol id) noexcept {
  if (id == no_action) return std::nullopt;
  const auto it = _registry.find(id);
  return it != _registry.end() ? std::optional<std::string_view>{it->second} : std::nullopt;
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

  friend void from_json(unmarshal::value json, offset& out) {
    out.x = unmarshal::get<float>(json, "x");
    out.y = unmarshal::get<float>(json, "y");
  }
};

struct frame final {
  int duration;
  offset offset;
  quad quad;

  friend void from_json(unmarshal::value json, frame& out) {
    out.duration = unmarshal::get<int64_t>(json, "duration");
    if (unmarshal::contains(json, "offset")) {
      out.offset = unmarshal::make<struct offset>(json["offset"]);
    }
    out.quad = unmarshal::make<struct quad>(json["quad"]);
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

inline void from_json(unmarshal::value json, b2AABB& out) {
  const auto x = unmarshal::get<float>(json, "x");
  const auto y = unmarshal::get<float>(json, "y");
  const auto w = unmarshal::get<float>(json, "w");
  const auto h = unmarshal::get<float>(json, "h");

  out.lowerBound = b2Vec2(x - epsilon, y - epsilon);
  out.upperBound = b2Vec2(x + w + epsilon, y + h + epsilon);
}

struct timeline final {
  bool oneshot{false};
  symbol next{no_action};
  std::optional<b2AABB> hitbox;
  boost::container::small_vector<frame, 24> frames;

  friend void from_json(unmarshal::value json, timeline& out) {
    out.oneshot = unmarshal::value_or(json, "oneshot", false);

    if (unmarshal::contains(json, "next")) {
      out.next = _resolve(unmarshal::get<std::string_view>(json, "next"));
    }

    if (auto opt = unmarshal::find_object(json, "hitbox")) {
      auto& h = *opt;
      if (auto q = h["quad"]; !q.error()) {
        out.hitbox = unmarshal::make<b2AABB>(q);
      }
    }

    if (auto array = unmarshal::find_array(json, "frames")) {
      for (auto element : *array) {
        out.frames.emplace_back(unmarshal::make<frame>(element));
      }
    }
  }
};

struct atlas final {
  entt::dense_map<symbol, timeline> timelines;

  const timeline* find(symbol id) const noexcept {
    const auto it = timelines.find(id);
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
  symbol action{no_action};
  const timeline* timeline{nullptr};
};

struct renderable final {
  int z;
  bool visible{true};
};

struct metadata final {
  symbol kind{no_action};
};

struct orientation final {
  flip flip{flip::none};
};

struct callbacks {
  functor on_mail;
  functor on_hover;
  functor on_unhover;
  functor on_touch;
  functor on_begin;
  functor on_end;

  std::shared_ptr<entityproxy> self;
};
