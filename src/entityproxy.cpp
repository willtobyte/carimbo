#include "entityproxy.hpp"

#include "physics.hpp"

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

symbol entityproxy::action() const noexcept {
  const auto& s = _registry.get<playback>(_entity);
  return s.action;
}

void entityproxy::set_action(symbol id) noexcept {
  auto& s = _registry.get<playback>(_entity);
  s.action = id;
  s.current_frame = 0;
  s.dirty = true;
  s.timeline = nullptr;
}

symbol entityproxy::kind() const noexcept {
  const auto& m = _registry.get<metadata>(_entity);
  return m.kind;
}

void entityproxy::set_kind(symbol id) noexcept {
  auto& m = _registry.get<metadata>(_entity);
  m.kind = id;
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

void entityproxy::set_onmail(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_mail = std::move(fn);
}

void entityproxy::set_onhover(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_hover = std::move(fn);
}

void entityproxy::set_onunhover(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_unhover = std::move(fn);
}

void entityproxy::set_ontouch(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_touch = std::move(fn);
}

void entityproxy::set_onbegin(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_begin = std::move(fn);
}

void entityproxy::set_onend(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_end = std::move(fn);
}

void entityproxy::set_oncollision(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_collision = std::move(fn);
}

void entityproxy::set_oncollisionend(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_collision_end = std::move(fn);
}

void entityproxy::set_ontick(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_entity);
  c.on_tick = std::move(fn);
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

std::shared_ptr<entityproxy> entityproxy::clone() {
  const auto e = _registry.create();

  auto [m, tn, sp, pb, tf, at, ori, rn] = _registry.try_get<metadata, tint, sprite, playback, transform, std::shared_ptr<const atlas>, orientation, renderable>(_entity);

  if (m) {
    _registry.emplace<metadata>(e, *m);
  }

  if (tn) {
    _registry.emplace<tint>(e, *tn);
  }

  if (sp) {
    _registry.emplace<sprite>(e, *sp);
  }

  if (pb) {
    playback cpb = *pb;
    cpb.dirty = true;
    cpb.redraw = true;
    _registry.emplace<playback>(e, std::move(cpb));
  }

  if (tf) {
    _registry.emplace<transform>(e, *tf);
  }

  if (at) {
    _registry.emplace<std::shared_ptr<const atlas>>(e, *at);
  }

  if (ori) {
    _registry.emplace<orientation>(e, *ori);
  }

  _registry.emplace<rigidbody>(e);

  if (rn) {
    rn->z = rn->z + 1;
    _registry.emplace<renderable>(e, *rn);
  }

  const auto proxy = std::make_shared<entityproxy>(e, _registry);

  _registry.emplace<callbacks>(e);

  return proxy;
}
