#include "components.hpp"

#include "objectproxy.hpp"

interning::counter::counter() {
  _counters.reserve(8);
}

uint32_t interning::counter::operator()(symbol id) {
  return ++_counters[id];
}

interning::interning() {
  _symbols.reserve(64);
  _bytecodes.reserve(128);
}

symbol interning::intern(std::string_view value) {
  if (value.empty()) [[unlikely]] return empty;
  const auto id = entt::hashed_string{value.data(), value.size()}.value();
  _symbols.try_emplace(id, value);
  return id;
}

std::string_view interning::lookup(symbol id) const noexcept {
  const auto it = _symbols.find(id);
  assert(it != _symbols.end() && "symbol not found in interning table");
  return it->second;
}

void scripting::wire(entt::entity entity, sol::environment& parent,
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

  derive(entity, parent, std::move(proxy), code, id);
}

void scripting::derive(entt::entity entity, sol::environment& parent,
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
