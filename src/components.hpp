#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"

using symbol = entt::id_type;
inline constexpr symbol empty = 0;

class objectproxy;

class interning final {
  class counter final {
  public:
    counter();
    [[nodiscard]] uint32_t operator()(symbol id);

  private:
    boost::unordered_flat_map<symbol, uint32_t> _counters;
  };

public:
  interning();

  [[nodiscard]] symbol intern(std::string_view value);
  [[nodiscard]] std::string_view lookup(symbol id) const noexcept;

  template <typename F>
  [[nodiscard]] std::shared_ptr<const std::string> bytecode(symbol id, F&& factory) {
    auto [it, inserted] = _bytecodes.try_emplace(id, nullptr);
    if (inserted) it->second = factory();
    return it->second;
  }

  counter increment;

private:
  boost::unordered_flat_map<symbol, std::string> _symbols{{empty, {}}};
  boost::unordered_flat_map<symbol, std::shared_ptr<const std::string>> _bytecodes;
};

struct transform final {
	vec2 position;
	double angle;
	float scale;
};

struct velocity final {
  vec2 value{0, 0};
};

struct tint final {
  uint8_t r{0};
  uint8_t g{0};
  uint8_t b{0};
  uint8_t a{255};
};

struct frame final {
  int32_t duration;
  float offset_x;
  float offset_y;
  quad quad;

  frame(unmarshal::json node) noexcept
      : duration(node["duration"].get<int32_t>()),
        offset_x(node["offset"]["x"].get(0.f)),
        offset_y(node["offset"]["y"].get(0.f)),
        quad(node["quad"]["x"].get<float>(),
             node["quad"]["y"].get<float>(),
             node["quad"]["w"].get<float>(),
             node["quad"]["h"].get<float>()) {}
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

struct timeline final {
  bool oneshot{false};
  symbol next{empty};
  std::optional<b2AABB> hitbox;
  boost::container::small_vector<frame, 24> frames;
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
  symbol chunkname{};
  functor on_spawn;
  functor on_dispose;
  functor on_loop;
};

class scripting final {
public:
  explicit scripting(entt::registry& registry) noexcept
      : _registry(registry) {}

  void wire(entt::entity entity, sol::environment& parent,
            std::shared_ptr<objectproxy> proxy, std::string_view filename);

  void derive(entt::entity entity, sol::environment& parent,
              std::shared_ptr<objectproxy> proxy,
              std::shared_ptr<const std::string> bytecode, symbol chunkname);

private:
  entt::registry& _registry;
};
