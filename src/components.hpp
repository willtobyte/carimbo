#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

using action_id = entt::id_type;
inline constexpr action_id no_action = 0;

[[nodiscard]] inline boost::unordered_flat_map<action_id, std::string>& action_registry() noexcept {
  static boost::unordered_flat_map<action_id, std::string> registry;
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
      unmarshal::value value;
      json["offset"].get(value);
      from_json(value, out.offset);
    }
    unmarshal::value value;
    json["quad"].get(value);
    from_json(value, out.quad);
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
  action_id next{no_action};
  std::optional<b2AABB> hitbox;
  boost::container::small_vector<frame, 24> frames;

  friend void from_json(unmarshal::value json, timeline& out) {
    out.oneshot = unmarshal::value_or(json, "oneshot", false);

    if (unmarshal::contains(json, "next")) {
      out.next = make_action(unmarshal::get<std::string_view>(json, "next"));
    }

    if (auto opt = unmarshal::find_object(json, "hitbox")) {
      auto& h = *opt;
      if (auto quad = h["quad"]; !quad.error()) {
        auto aabb = b2AABB{};
        unmarshal::value value;
        quad.get(value);
        from_json(value, aabb);
        out.hitbox = aabb;
      }
    }

    if (auto array = unmarshal::find_array(json, "frames")) {
      for (auto element : *array) {
        auto f = frame{};
        unmarshal::value value;
        element.get(value);
        from_json(value, f);
        out.frames.emplace_back(std::move(f));
      }
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
  functor on_begin;
  functor on_end;

  std::shared_ptr<entityproxy> self;
};
