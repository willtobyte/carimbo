#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

using symbol = entt::id_type;
inline constexpr symbol empty = 0;

inline boost::unordered_flat_map<symbol, std::string> symbols{{empty, {}}};

[[nodiscard]] inline symbol intern(std::string_view value) noexcept {
  if (value.empty()) [[unlikely]] return empty;
  const auto id = entt::hashed_string{value.data(), value.size()}.value();
  symbols.try_emplace(id, value);
  return id;
}

[[nodiscard]] inline std::string_view lookup(symbol id) noexcept {
  return symbols.find(id)->second;
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

  void decode(unmarshal::value node) noexcept {
    x = unmarshal::get<float>(node, "x");
    y = unmarshal::get<float>(node, "y");
  }
};

struct frame final {
  int duration;
  offset offset;
  quad quad;

  void decode(unmarshal::value node) noexcept {
    duration = unmarshal::get<int64_t>(node, "duration");
    unmarshal::into(node, "offset", offset);
    quad = unmarshal::make<struct quad>(unmarshal::child(node, "quad"));
  }
};

enum class bodytype : uint8_t {
  static_body = 0,
  kinematic,
  dynamic
};

struct rigidbody final {
  struct state {
    b2Vec2 position{};
    b2Rot rotation{};
    float hx{};
    float hy{};

    [[nodiscard]] bool update_transform(const state& other) noexcept {
      if (position.x == other.position.x && position.y == other.position.y &&
          rotation.c == other.rotation.c && rotation.s == other.rotation.s)
        return false;
      position = other.position;
      rotation = other.rotation;
      return true;
    }

    [[nodiscard]] bool update_shape(const state& other) noexcept {
      if (hx == other.hx && hy == other.hy)
        return false;
      hx = other.hx;
      hy = other.hy;
      return true;
    }
  };

  b2BodyId body;
  b2ShapeId shape;
  state cache;
  bodytype type{bodytype::kinematic};
  bool enabled{true};

  bool is_valid() const noexcept {
    return b2Body_IsValid(body);
  }
};

inline void decode(unmarshal::value node, b2AABB& out) noexcept {
  const auto x = unmarshal::get<float>(node, "x");
  const auto y = unmarshal::get<float>(node, "y");
  const auto w = unmarshal::get<float>(node, "w");
  const auto h = unmarshal::get<float>(node, "h");

  out.lowerBound = b2Vec2(x - epsilon, y - epsilon);
  out.upperBound = b2Vec2(x + w + epsilon, y + h + epsilon);
}

struct timeline final {
  bool oneshot{false};
  symbol next{empty};
  std::optional<b2AABB> hitbox;
  boost::container::small_vector<frame, 24> frames;

  void decode(unmarshal::value node) noexcept {
    oneshot = unmarshal::get_or(node, "oneshot", false);

    if (auto nextval = unmarshal::find<std::string_view>(node, "next")) {
      next = intern(*nextval);
    }

    if (auto hitboxval = unmarshal::child(node, "hitbox")) {
      if (auto aabb = unmarshal::child(hitboxval, "aabb")) {
        hitbox = unmarshal::make<b2AABB>(aabb);
      }
    }

    unmarshal::collect<frame>(node, "frames", frames);
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
  symbol action{empty};
  const timeline* timeline{nullptr};
};

struct renderable final {
  int z;
  bool visible{true};
};

struct metadata final {
  symbol kind{empty};
  symbol name{empty};
};

struct orientation final {
  flip flip{flip::none};
};

struct hoverable {
  functor on_hover;
  functor on_unhover;
};

struct touchable {
  functor on_touch;
};

struct animatable {
  functor on_begin;
  functor on_end;
};

struct collidable {
  functor on_collision;
  functor on_collision_end;
};

struct tickable {
  functor on_tick;
};

struct scriptable {
  sol::environment parent;
  sol::environment environment;
  sol::table module;
  std::shared_ptr<const std::string> bytecode;
  functor on_spawn;
  functor on_dispose;
  functor on_loop;
};
