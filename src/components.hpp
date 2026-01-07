#pragma once

#include "common.hpp"

#include "geometry.hpp"
#include "flip.hpp"
#include "io.hpp"

using symbol = entt::id_type;
inline constexpr symbol empty = 0;

class objectproxy;

class interning final {
  class counter final {
  public:
    counter() noexcept { _counters.reserve(8); }

    [[nodiscard]] uint32_t operator()(symbol id) noexcept {
      return ++_counters[id];
    }

  private:
    boost::unordered_flat_map<symbol, uint32_t> _counters;
  };

public:
  interning() noexcept {
    _symbols.reserve(64);
    _bytecodes.reserve(128);
  }

  [[nodiscard]] symbol intern(std::string_view value) noexcept {
    if (value.empty()) [[unlikely]] return empty;
    const auto id = entt::hashed_string{value.data(), value.size()}.value();
    _symbols.try_emplace(id, value);
    return id;
  }

  [[nodiscard]] std::string_view lookup(symbol id) const noexcept {
    return _symbols.find(id)->second;
  }

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

  void attach(entt::entity entity, sol::environment& parent,
              std::shared_ptr<objectproxy> proxy, std::string_view filename) {
    if (!io::exists(filename)) return;

    auto& interning = _registry.ctx().get<::interning>();
    const auto id = interning.intern(filename);
    const auto code = interning.bytecode(id, [&] {
      sol::state_view lua(parent.lua_state());
      const auto buffer = io::read(filename);

      const auto result = lua.load(
        std::string_view{reinterpret_cast<const char*>(buffer.data()), buffer.size()},
        std::format("@{}", filename)
      );

      verify(result);

      auto function = result.get<sol::protected_function>();

      std::string bytecode;
      bytecode.reserve(buffer.size() * 7 / 2);
      function.push();
      lua_dump(lua.lua_state(), [](lua_State*, const void* data, size_t size, void* userdata) -> int {
        static_cast<std::string*>(userdata)->append(static_cast<const char*>(data), size);
        return 0;
      }, &bytecode, 0);
      lua_pop(lua.lua_state(), 1);

      return std::make_shared<const std::string>(std::move(bytecode));
    });

    attach(entity, parent, std::move(proxy), code, id);
  }

  void attach(entt::entity entity, sol::environment& parent,
              std::shared_ptr<objectproxy> proxy,
              std::shared_ptr<const std::string> bytecode, symbol chunkname) {
    const auto& interning = _registry.ctx().get<::interning>();
    sol::state_view lua(parent.lua_state());
    sol::environment environment(lua, sol::create, parent);
    environment["self"] = std::move(proxy);

    const auto result = lua.load(*bytecode, std::format("@{}", interning.lookup(chunkname)));
    verify(result);

    auto function = result.get<sol::protected_function>();
    sol::set_environment(environment, function);

    const auto exec = function();
    verify(exec);

    auto module = exec.get<sol::table>();

    scriptable sc;
    sc.parent = parent;
    sc.environment = environment;
    sc.module = module;
    sc.bytecode = std::move(bytecode);
    sc.chunkname = chunkname;

    if (auto fn = module["on_spawn"].get<sol::protected_function>(); fn.valid()) {
      sc.on_spawn = std::move(fn);
    }

    if (auto fn = module["on_dispose"].get<sol::protected_function>(); fn.valid()) {
      sc.on_dispose = std::move(fn);
    }

    if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
      sc.on_loop = std::move(fn);
    }

    if (auto on_begin = module["on_begin"].get<sol::protected_function>(),
           on_end = module["on_end"].get<sol::protected_function>();
        on_begin.valid() || on_end.valid()) {
      auto& a = _registry.emplace<animatable>(entity);
      a.on_begin = std::move(on_begin);
      a.on_end = std::move(on_end);
    }

    if (auto on_collision = module["on_collision"].get<sol::protected_function>(),
           on_collision_end = module["on_collision_end"].get<sol::protected_function>();
        on_collision.valid() || on_collision_end.valid()) {
      auto& c = _registry.emplace<collidable>(entity);
      c.on_collision = std::move(on_collision);
      c.on_collision_end = std::move(on_collision_end);
    }

    if (auto on_hover = module["on_hover"].get<sol::protected_function>(),
           on_unhover = module["on_unhover"].get<sol::protected_function>();
        on_hover.valid() || on_unhover.valid()) {
      auto& h = _registry.emplace<hoverable>(entity);
      h.on_hover = std::move(on_hover);
      h.on_unhover = std::move(on_unhover);
    }

    if (auto fn = module["on_touch"].get<sol::protected_function>(); fn.valid()) {
      _registry.emplace<touchable>(entity, std::move(fn));
    }

    _registry.emplace<scriptable>(entity, std::move(sc));
  }

private:
  entt::registry& _registry;
};
