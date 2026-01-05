#include "entityproxy.hpp"

#include "physics.hpp"

static boost::unordered_flat_map<symbol, uint32_t> counters;

entityproxy::entityproxy(entt::entity entity, entt::registry& registry) noexcept
  : _entity(entity), _registry(registry) {
}

uint64_t entityproxy::id() const noexcept {
  return static_cast<uint64_t>(_entity);
}

float entityproxy::x() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position.x;
}

void entityproxy::set_x(float x) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position.x = x;
}

float entityproxy::y() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position.y;
}

void entityproxy::set_y(float y) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position.y = y;
}

vec2 entityproxy::position() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.position;
}

void entityproxy::set_position(const vec2& position) noexcept {
  auto& t = _registry.get<transform>(_entity);
  t.position = position;
}

uint8_t entityproxy::alpha() const noexcept {
  const auto& t = _registry.get<tint>(_entity);
  return t.a;
}

void entityproxy::set_alpha(uint8_t alpha) noexcept {
  auto [t, s] = _registry.get<tint, playback>(_entity);
  t.a = alpha;
  s.redraw = true;
}

double entityproxy::angle() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.angle;
}

void entityproxy::set_angle(double angle) noexcept {
  auto [t, s] = _registry.get<transform, playback>(_entity);
  t.angle = angle;
  s.redraw = true;
}

float entityproxy::scale() const noexcept {
  const auto& t = _registry.get<transform>(_entity);
  return t.scale;
}

void entityproxy::set_scale(float scale) noexcept {
  auto [t, s] = _registry.get<transform, playback>(_entity);
  t.scale = scale;
  s.dirty = true;
  s.redraw = true;
}

bool entityproxy::visible() const noexcept {
  const auto& r = _registry.get<renderable>(_entity);
  return r.visible;
}

void entityproxy::set_visible(bool visible) noexcept {
  auto& r = _registry.get<renderable>(_entity);
  r.visible = visible;
}

std::string_view entityproxy::action() const noexcept {
  const auto& s = _registry.get<playback>(_entity);
  return lookup(s.action);
}

void entityproxy::set_action(std::string_view value) noexcept {
  auto& s = _registry.get<playback>(_entity);
  s.action = intern(value);
  s.current_frame = 0;
  s.dirty = true;
  s.timeline = nullptr;
}

std::string_view entityproxy::kind() const noexcept {
  const auto& m = _registry.get<metadata>(_entity);
  return lookup(m.kind);
}

void entityproxy::set_kind(std::string_view value) noexcept {
  auto& m = _registry.get<metadata>(_entity);
  m.kind = intern(value);
}

flip entityproxy::flip() const noexcept {
  const auto& o = _registry.get<::orientation>(_entity);
  return o.flip;
}

void entityproxy::set_flip(::flip flip) noexcept {
  auto [o, s] = _registry.get<::orientation, playback>(_entity);
  o.flip = flip;
  s.redraw = true;
}

int entityproxy::z() const noexcept {
  const auto& r = _registry.get<renderable>(_entity);
  return r.z;
}

void entityproxy::set_z(int value) noexcept {
  auto& r = _registry.get<renderable>(_entity);

  if (r.z == value) return;

  r.z = value;

  _registry.sort<renderable>([](const renderable& lhs, const renderable& rhs) {
    return lhs.z < rhs.z;
  });
}

void entityproxy::set_onhover(sol::protected_function fn) {
  auto& h = _registry.get_or_emplace<hoverable>(_entity);
  h.on_hover = std::move(fn);
}

void entityproxy::set_onunhover(sol::protected_function fn) {
  auto& h = _registry.get_or_emplace<hoverable>(_entity);
  h.on_unhover = std::move(fn);
}

void entityproxy::set_ontouch(sol::protected_function fn) {
  auto& t = _registry.emplace_or_replace<touchable>(_entity);
  t.on_touch = std::move(fn);
}

void entityproxy::set_onbegin(sol::protected_function fn) {
  auto& a = _registry.get_or_emplace<animatable>(_entity);
  a.on_begin = std::move(fn);
}

void entityproxy::set_onend(sol::protected_function fn) {
  auto& a = _registry.get_or_emplace<animatable>(_entity);
  a.on_end = std::move(fn);
}

void entityproxy::set_oncollision(sol::protected_function fn) {
  auto& c = _registry.get_or_emplace<collidable>(_entity);
  c.on_collision = std::move(fn);
}

void entityproxy::set_oncollisionend(sol::protected_function fn) {
  auto& c = _registry.get_or_emplace<collidable>(_entity);
  c.on_collision_end = std::move(fn);
}

void entityproxy::set_ontick(sol::protected_function fn) {
  auto& t = _registry.emplace_or_replace<tickable>(_entity);
  t.on_tick = std::move(fn);
}

bool entityproxy::alive() const noexcept {
  return _registry.valid(_entity);
}

void entityproxy::die() noexcept {
  if (!_registry.valid(_entity)) [[unlikely]] return;

  if (auto* r = _registry.try_get<rigidbody>(_entity)) {
    physics::destroy(r->shape, r->body);
  }

  _registry.destroy(_entity);
}

std::string_view entityproxy::name() const noexcept {
  const auto& m = _registry.get<metadata>(_entity);
  return lookup(m.name);
}

std::shared_ptr<entityproxy> entityproxy::clone() {
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

  const auto proxy = std::make_shared<entityproxy>(entity, _registry);

  if (sc && sc->bytecode) {
    sol::state_view lua(sc->parent.lua_state());
    sol::environment environment(lua, sol::create, sc->parent);
    environment["self"] = proxy;

    const auto result = lua.load(*sc->bytecode, "@clone");
    verify(result);

    auto function = result.get<sol::protected_function>();
    sol::set_environment(environment, function);

    const auto exec = function();
    verify(exec);

    auto module = exec.get<sol::table>();

    scriptable clone;
    clone.parent = sc->parent;
    clone.environment = environment;
    clone.module = module;
    clone.bytecode = sc->bytecode;

    if (auto fn = module["on_spawn"].get<sol::protected_function>(); fn.valid()) {
      clone.on_spawn = std::move(fn);
    }

    if (auto fn = module["on_dispose"].get<sol::protected_function>(); fn.valid()) {
      clone.on_dispose = std::move(fn);
    }

    if (auto fn = module["on_loop"].get<sol::protected_function>(); fn.valid()) {
      clone.on_loop = std::move(fn);
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

    _registry.emplace<scriptable>(entity, std::move(clone));
  }

  return proxy;
}
