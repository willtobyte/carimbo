#include "objectproxy.hpp"

#include "components.hpp"
#include "physics.hpp"

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

vec2 objectproxy::velocity() const noexcept {
  const auto& v = _registry.get<::velocity>(_entity);
  return v.value;
}

void objectproxy::set_velocity(const vec2& vel) noexcept {
  auto& v = _registry.get<::velocity>(_entity);
  v.value = vel;
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
  const auto& interning = _registry.ctx().get<::interning>();
  const auto& s = _registry.get<playback>(_entity);
  return interning.lookup(s.action);
}

void objectproxy::set_action(std::string_view value) {
  auto& interning = _registry.ctx().get<::interning>();
  auto [s, at] = _registry.get<playback, std::shared_ptr<const atlas>>(_entity);
  s.action = interning.intern(value);
  s.current_frame = 0;
  s.dirty = false;
  s.timeline = at->find(s.action);
}

std::string_view objectproxy::kind() const noexcept {
  const auto& interning = _registry.ctx().get<::interning>();
  const auto& m = _registry.get<metadata>(_entity);
  return interning.lookup(m.kind);
}

void objectproxy::set_kind(std::string_view value) {
  auto& interning = _registry.ctx().get<::interning>();
  auto& m = _registry.get<metadata>(_entity);
  m.kind = interning.intern(value);
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
  auto& state = _registry.ctx().get<renderstate>();
  state.set_z(r, value);
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
  auto& t = _registry.get_or_emplace<touchable>(_entity);
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
  auto& t = _registry.get_or_emplace<tickable>(_entity);
  t.on_tick = std::move(fn);
}

bool objectproxy::alive() const noexcept {
  return _registry.valid(_entity);
}

void objectproxy::die() noexcept {
  if (!_registry.valid(_entity)) [[unlikely]] return;

  _registry.destroy(_entity);
}

std::string_view objectproxy::name() const noexcept {
  const auto& interning = _registry.ctx().get<::interning>();
  const auto& m = _registry.get<metadata>(_entity);
  return interning.lookup(m.name);
}

std::shared_ptr<objectproxy> objectproxy::clone() {
  const auto entity = _registry.create();

  auto [m, tn, sp, pb, tf, at, ori, rn, sc] = _registry.try_get<metadata, tint, sprite, playback, transform, std::shared_ptr<const atlas>, orientation, renderable, scriptable>(_entity);

  if (m) {
    auto& interning = _registry.ctx().get<::interning>();
    metadata cp = *m;
    const auto original = interning.lookup(m->name);
    cp.name = interning.intern(std::format("{}_{}", original, interning.increment(m->name)));

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

  const auto position = tf ? tf->position : vec2{0, 0};
  auto* world = _registry.ctx().get<physics::world*>();
  _registry.emplace<physics::body>(entity, physics::body::create(*world, {.type = physics::bodytype::kinematic, .position = position, .entity = entity}));
  _registry.emplace<struct velocity>(entity);

  if (rn) {
    renderable copy = *rn;
    copy.z = rn->z + 1;
    _registry.emplace<renderable>(entity, std::move(copy));
  }

  auto proxy = std::make_shared<objectproxy>(entity, _registry);

  if (sc && sc->bytecode) {
    auto& scripting = _registry.ctx().get<::scripting>();
    scripting.derive(entity, sc->parent, proxy, sc->bytecode, sc->chunkname);
  }

  return proxy;
}
