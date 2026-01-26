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
        quad(node["quad"].get<::quad>()) {}
};

struct timeline final {
  bool oneshot{false};
  symbol next{empty};
  std::optional<quad> hitbox;
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
  pixmap* pixmap;
};

struct playback final {
  uint16_t current_frame{0};
  uint64_t tick{0};
  symbol action{empty};
  const timeline* timeline{nullptr};
};

struct dirtable final {
  uint8_t flags{0xff};

  static constexpr uint8_t animation = 1 << 0;
  static constexpr uint8_t physics = 1 << 1;
  static constexpr uint8_t render = 1 << 2;

  void mark(uint8_t f) noexcept { flags |= f; }
  void clear(uint8_t f) noexcept { flags &= ~f; }
  [[nodiscard]] bool is(uint8_t f) const noexcept { return flags & f; }
};

struct drawable final {
  float x{0};
  float y{0};
  float w{0};
  float h{0};
};

enum class renderablekind : uint8_t { sprite, particle };

struct particlerenderable final {
  std::shared_ptr<particlebatch> batch;
};

struct renderable final {
  int z;
  bool visible{true};
  renderablekind kind{renderablekind::sprite};
};

struct renderstate final {
  bool z_dirty{false};

  void set_z(renderable& r, int value) {
    if (r.z == value) [[likely]] return;

    r.z = value;
    z_dirty = true;
  }

  void flush(entt::registry& registry) {
    if (!z_dirty) [[likely]] return;

    registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
      return lhs.z < rhs.z;
    });

    z_dirty = false;
  }
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

  void wire(entt::entity entity, sol::environment& parent, std::shared_ptr<objectproxy> proxy, std::string_view filename);

  void derive(entt::entity entity, sol::environment& parent, std::shared_ptr<objectproxy> proxy, std::shared_ptr<const std::string> bytecode, symbol chunkname);

private:
  entt::registry& _registry;
};
