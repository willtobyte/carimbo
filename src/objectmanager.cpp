#include "objectmanager.hpp"

#include "components.hpp"
#include "entityproxy.hpp"
#include "io.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

objectmanager::objectmanager(
    entt::registry& registry,
    std::shared_ptr<renderer> renderer,
    std::string_view scenename,
    sol::environment& environment
)
    : _registry(registry),
      _renderer(std::move(renderer)),
      _scenename(scenename),
      _environment(environment) {
  _proxies.reserve(32);
}

void objectmanager::add(unmarshal::json node, int32_t z) {
  const auto name = node["name"].get<std::string_view>();
  const auto kind = node["kind"].get<std::string_view>();
  const auto action = intern(node["action"].get<std::string_view>());
  const auto x = node["x"].get<float>();
  const auto y = node["y"].get<float>();

  auto json = unmarshal::parse(io::read(std::format("objects/{}/{}.json", _scenename, kind)));

  const auto entity = _registry.create();

  auto at = std::make_shared<atlas>();
  if (auto timelines = json["timelines"]) {
    timelines.foreach([&at](std::string_view key, unmarshal::json node) {
      timeline tl{};
      tl.oneshot = node["oneshot"].get(false);

      if (auto nextnode = node["next"]) {
        tl.next = intern(nextnode.get<std::string_view>());
      }

      if (auto hitboxval = node["hitbox"]) {
        if (auto aabb = hitboxval["aabb"]) {
          const auto hx = aabb["x"].get<float>();
          const auto hy = aabb["y"].get<float>();
          const auto hw = aabb["w"].get<float>();
          const auto hh = aabb["h"].get<float>();
          tl.hitbox = b2AABB{
            .lowerBound = b2Vec2(hx - epsilon, hy - epsilon),
            .upperBound = b2Vec2(hx + hw + epsilon, hy + hh + epsilon)
          };
        }
      }

      node["frames"].foreach([&tl](unmarshal::json f) {
        tl.frames.emplace_back(f);
      });

      at->timelines.emplace(intern(key), std::move(tl));
    });
  }

  _registry.emplace<std::shared_ptr<const atlas>>(entity, std::move(at));

  metadata md{
    .kind = intern(kind),
    .name = intern(name)
  };
  _registry.emplace<metadata>(entity, md);

  _registry.emplace<tint>(entity);

  sprite sp{
      .pixmap = std::make_shared<pixmap>(_renderer, std::format("blobs/{}/{}.png", _scenename, kind))};
  _registry.emplace<sprite>(entity, std::move(sp));

  playback pb{
    .dirty = true,
    .redraw = false,
    .current_frame = 0,
    .tick = SDL_GetTicks(),
    .action = action,
    .timeline = nullptr
  };
  _registry.emplace<playback>(entity, std::move(pb));

  const auto scale = json["scale"].get(1.0f);

  transform tr{
    .position = vec2{x, y},
    .angle = .0,
    .scale = scale
  };
  _registry.emplace<transform>(entity, std::move(tr));

  _registry.emplace<orientation>(entity);

  _registry.emplace<rigidbody>(entity);

  renderable rd{
    .z = z
  };
  _registry.emplace<renderable>(entity, std::move(rd));

  const auto filename = std::format("objects/{}/{}.lua", _scenename, kind);

  const auto proxy = std::make_shared<entityproxy>(entity, _registry);

  _proxies.emplace(name, proxy);

  if (io::exists(filename)) {
    sol::state_view lua(_environment.lua_state());
    sol::environment environment(lua, sol::create, _environment);
    environment["self"] = proxy;

    const auto buffer = io::read(filename);
    std::string_view source{reinterpret_cast<const char*>(buffer.data()), buffer.size()};

    const auto result = lua.load(source, std::format("@{}", filename));
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

    sol::set_environment(environment, function);

    const auto exec = function();
    verify(exec);

    auto module = exec.get<sol::table>();

    scriptable sc;
    sc.parent = _environment;
    sc.environment = environment;
    sc.module = module;
    sc.bytecode = std::make_shared<const std::string>(std::move(bytecode));

    if (auto fn = module["on_spawn"].get<sol::protected_function>(); fn.valid()) {
      sc.on_spawn = std::move(fn);
    }

    if (auto fn = module["on_dispose"].get<sol::protected_function>(); fn.valid()) {
      sc.on_dispose = std::move(fn);
    }

    if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
      sc.on_loop = std::move(fn);
    }

    auto on_begin = module["on_begin"].get<sol::protected_function>();
    auto on_end = module["on_end"].get<sol::protected_function>();
    if (on_begin.valid() || on_end.valid()) {
      auto& a = _registry.emplace<animatable>(entity);
      a.on_begin = std::move(on_begin);
      a.on_end = std::move(on_end);
    }

    auto on_collision = module["on_collision"].get<sol::protected_function>();
    auto on_collision_end = module["on_collision_end"].get<sol::protected_function>();
    if (on_collision.valid() || on_collision_end.valid()) {
      auto& c = _registry.emplace<collidable>(entity);
      c.on_collision = std::move(on_collision);
      c.on_collision_end = std::move(on_collision_end);
    }

    auto on_hover = module["on_hover"].get<sol::protected_function>();
    auto on_unhover = module["on_unhover"].get<sol::protected_function>();
    if (on_hover.valid() || on_unhover.valid()) {
      auto& h = _registry.emplace<hoverable>(entity);
      h.on_hover = std::move(on_hover);
      h.on_unhover = std::move(on_unhover);
    }

    if (auto fn = module["on_touch"].get<sol::protected_function>(); fn.valid()) {
      _registry.emplace<touchable>(entity, std::move(fn));
    }

    _registry.emplace<scriptable>(entity, std::move(sc));
  }
}

void objectmanager::populate(sol::table& pool) const {
  for (const auto& [name, proxy] : _proxies) {
    assert(!pool[name].valid() && "duplicate key in pool");
    pool[name] = proxy;
  }
}

void objectmanager::sort() {
  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });
}
