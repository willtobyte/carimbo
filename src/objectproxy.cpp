#include "objectproxy.hpp"

#include "physics.hpp"

static boost::unordered_flat_map<symbol, uint32_t> counters;

objectproxy::objectproxy(entt::entity entity, entt::registry& registry) noexcept
  : _entity(entity), _registry(registry) {
}

uint64_t objectproxy::id() const noexcept {
  return static_cast<uint64_t>(_entity);
}

float objectproxy::x() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position.x;
}

void objectproxy::set_x(float x) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position.x = x;
}

float objectproxy::y() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position.y;
}

void objectproxy::set_y(float y) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position.y = y;
}

vec2 objectproxy::position() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position;
}

void objectproxy::set_position(const vec2& position) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position = position;
}

uint8_t objectproxy::alpha() const noexcept {
  const auto& t = _registry.get<tint>(_entity);
  return t.a;
}

void objectproxy::set_alpha(uint8_t alpha) noexcept {
  auto [t, s] = _registry.get<tint, playback>(_entity);
  t.a = alpha;
  s.redraw = true;
}

double objectproxy::angle() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.angle;
}

void objectproxy::set_angle(double angle) noexcept {
  auto [t, s] = _registry.get<transform, playback>(_entity);
  t.angle = angle;
  s.redraw = true;
}

float objectproxy::scale() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.scale;
}

void objectproxy::set_scale(float scale) noexcept {
  auto [t, s] = _registry.get<transform, playback>(_entity);
  t.scale = scale;
  s.dirty = true;
  s.redraw = true;
}

bool objectproxy::visible() const noexcept {
  const auto& r = _registry.get<renderable>(_entity);
  return r.visible;
}

void objectproxy::set_visible(bool visible) noexcept {
  auto& r = _registry.get<renderable>(_entity);
  r.visible = visible;
}

std::string_view objectproxy::action() const noexcept {
  const auto& s = _registry.get<playback>(_entity);
  return lookup(s.action);
}

void objectproxy::set_action(std::string_view value) noexcept {
  auto& s = _registry.get<playback>(_entity);
  s.action = intern(value);
  s.current_frame = 0;
  s.dirty = true;
  s.timeline = nullptr;
}

std::string_view objectproxy::kind() const noexcept {
  const auto& m = _registry.get<metadata>(_entity);
  return lookup(m.kind);
}

void objectproxy::set_kind(std::string_view value) noexcept {
  auto& m = _registry.get<metadata>(_entity);
  m.kind = intern(value);
}

flip objectproxy::flip() const noexcept {
  const auto& o = _registry.get<::orientation>(_entity);
  return o.flip;
}

void objectproxy::set_flip(::flip flip) noexcept {
  auto [o, s] = _registry.get<::orientation, playback>(_entity);
  o.flip = flip;
  s.redraw = true;
}

int objectproxy::z() const noexcept {
  const auto& r = _registry.get<renderable>(_entity);
  return r.z;
}

void objectproxy::set_z(int value) noexcept {
  auto& r = _registry.get<renderable>(_entity);

  if (r.z == value) return;

  r.z = value;

  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });
}

void objectproxy::set_onhover(sol::protected_function fn) {
  auto& h = _registry.get_or_emplace<hoverable>(_entity);
  h.on_hover = std::move(fn);
}

void objectproxy::set_onunhover(sol::protected_function fn) {
  auto& h = _registry.get_or_emplace<hoverable>(_entity);
  h.on_unhover = std::move(fn);
}

void objectproxy::set_ontouch(sol::protected_function fn) {
  auto& t = _registry.emplace_or_replace<touchable>(_entity);
  t.on_touch = std::move(fn);
}

void objectproxy::set_onbegin(sol::protected_function fn) {
  auto& a = _registry.get_or_emplace<animatable>(_entity);
  a.on_begin = std::move(fn);
}

void objectproxy::set_onend(sol::protected_function fn) {
  auto& a = _registry.get_or_emplace<animatable>(_entity);
  a.on_end = std::move(fn);
}

void objectproxy::set_oncollision(sol::protected_function fn) {
  auto& c = _registry.get_or_emplace<collidable>(_entity);
  c.on_collision = std::move(fn);
}

void objectproxy::set_oncollisionend(sol::protected_function fn) {
  auto& c = _registry.get_or_emplace<collidable>(_entity);
  c.on_collision_end = std::move(fn);
}

void objectproxy::set_ontick(sol::protected_function fn) {
  auto& t = _registry.emplace_or_replace<tickable>(_entity);
  t.on_tick = std::move(fn);
}

bool objectproxy::alive() const noexcept {
  return _registry.valid(_entity);
}

void objectproxy::die() noexcept {
  if (!_registry.valid(_entity)) [[unlikely]] return;

  if (auto* r = _registry.try_get<rigidbody>(_entity)) {
    physics::destroy(r->shape, r->body);
  }

  _registry.destroy(_entity);
}

std::string_view objectproxy::name() const noexcept {
  const auto& m = _registry.get<metadata>(_entity);
  return lookup(m.name);
}

static boost::unordered_flat_map<symbol, std::shared_ptr<const std::string>> bytecodes;

static void hydrate(
    entt::registry& registry,
    entt::entity entity,
    sol::environment& parent,
    std::shared_ptr<objectproxy> proxy,
    std::shared_ptr<const std::string> bytecode,
    symbol chunkname
) {
  sol::state_view lua(parent.lua_state());
  sol::environment environment(lua, sol::create, parent);
  environment["self"] = std::move(proxy);

  const auto result = lua.load(*bytecode, std::format("@{}", lookup(chunkname)));
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
    auto& a = registry.emplace<animatable>(entity);
    a.on_begin = std::move(on_begin);
    a.on_end = std::move(on_end);
  }

  if (auto on_collision = module["on_collision"].get<sol::protected_function>(),
         on_collision_end = module["on_collision_end"].get<sol::protected_function>();
      on_collision.valid() || on_collision_end.valid()) {
    auto& c = registry.emplace<collidable>(entity);
    c.on_collision = std::move(on_collision);
    c.on_collision_end = std::move(on_collision_end);
  }

  if (auto on_hover = module["on_hover"].get<sol::protected_function>(),
         on_unhover = module["on_unhover"].get<sol::protected_function>();
      on_hover.valid() || on_unhover.valid()) {
    auto& h = registry.emplace<hoverable>(entity);
    h.on_hover = std::move(on_hover);
    h.on_unhover = std::move(on_unhover);
  }

  if (auto fn = module["on_touch"].get<sol::protected_function>(); fn.valid()) {
    registry.emplace<touchable>(entity, std::move(fn));
  }

  registry.emplace<scriptable>(entity, std::move(sc));
}

void attach(
    entt::registry& registry,
    entt::entity entity,
    sol::environment& parent,
    std::shared_ptr<objectproxy> proxy,
    std::string_view filename
) {
  if (!io::exists(filename)) return;

  const auto id = intern(filename);
  auto [it, inserted] = bytecodes.try_emplace(id, nullptr);
  if (inserted) {
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

    it->second = std::make_shared<const std::string>(std::move(bytecode));
  }

  hydrate(registry, entity, parent, std::move(proxy), it->second, id);
}

std::shared_ptr<objectproxy> objectproxy::clone() {
  const auto entity = _registry.create();

  auto [m, tn, sp, pb, tf, at, ori, rn, sc] = _registry.try_get<metadata, tint, sprite, playback, transform, std::shared_ptr<const atlas>, orientation, renderable, scriptable>(_entity);

  if (m) {
    metadata cp = *m;
    const auto original = lookup(m->name);
    cp.name = intern(std::format("{}_{}", original, ++counters[m->name]));

    _registry.emplace<metadata>(entity, cp);
  }

  if (tn) {
    _registry.emplace<tint>(entity, *tn);
  }

  if (sp) {
    _registry.emplace<sprite>(entity, *sp);
  }

  if (pb) {
    playback copy = *pb;
    copy.dirty = true;
    copy.redraw = true;
    _registry.emplace<playback>(entity, std::move(copy));
  }

  if (tf) {
    _registry.emplace<transform>(entity, *tf);
  }

  if (at) {
    _registry.emplace<std::shared_ptr<const atlas>>(entity, *at);
  }

  if (ori) {
    _registry.emplace<orientation>(entity, *ori);
  }

  _registry.emplace<rigidbody>(entity);

  if (rn) {
    renderable copy = *rn;
    copy.z = rn->z + 1;
    _registry.emplace<renderable>(entity, std::move(copy));
  }

  auto proxy = std::make_shared<objectproxy>(entity, _registry);

  if (sc && sc->bytecode) {
    hydrate(_registry, entity, sc->parent, proxy, sc->bytecode, sc->chunkname);
  }

  return proxy;
}
