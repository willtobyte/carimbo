#include "entityproxy.hpp"
#include "components.hpp"
#include "helper.hpp"

entityproxy::entityproxy(entt::entity entity, entt::registry& registry) noexcept
  : _e(entity), _registry(registry) {
}

uint64_t entityproxy::id() const noexcept {
  return static_cast<uint64_t>(_e);
}

float entityproxy::x() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.position.x;
}

void entityproxy::set_x(float x) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.position.x = x;
}

float entityproxy::y() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.position.y;
}

void entityproxy::set_y(float y) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.position.y = y;
}

vec2 entityproxy::position() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.position;
}

void entityproxy::set_position(const vec2& position) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.position = position;
}

uint8_t entityproxy::alpha() const noexcept {
  const auto& t = _registry.get<tint>(_e);
  return t.a;
}

void entityproxy::set_alpha(uint8_t alpha) noexcept {
  auto& t = _registry.get<tint>(_e);
  t.a = alpha;

  auto& s = _registry.get<playback>(_e);
  s.redraw = true;
}

double entityproxy::angle() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.angle;
}

void entityproxy::set_angle(double angle) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.angle = angle;

  auto& s = _registry.get<playback>(_e);
  s.redraw = true;
}

float entityproxy::scale() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.scale;
}

void entityproxy::set_scale(float scale) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.scale = scale;

  auto& s = _registry.get<playback>(_e);
  s.dirty = true;
  s.redraw = true;
}

bool entityproxy::visible() const noexcept {
  const auto& r = _registry.get<renderable>(_e);
  return r.visible;
}

void entityproxy::set_visible(bool visible) noexcept {
  auto& r = _registry.get<renderable>(_e);
  r.visible = visible;

  auto& p = _registry.get<physics>(_e);
  p.dirty = true;
}

action_id entityproxy::action() const noexcept {
  const auto& s = _registry.get<playback>(_e);
  return s.action;
}

void entityproxy::set_action(action_id id) noexcept {
  auto& s = _registry.get<playback>(_e);
  s.action = id;
  s.current_frame = 0;
  s.dirty = true;
  s.timeline = nullptr;
}

action_id entityproxy::kind() const noexcept {
  const auto& m = _registry.get<metadata>(_e);
  return m.kind;
}

void entityproxy::set_kind(action_id id) noexcept {
  auto& m = _registry.get<metadata>(_e);
  m.kind = id;
}

flip entityproxy::flip() const noexcept {
  const auto& o = _registry.get<::orientation>(_e);
  return o.flip;
}

void entityproxy::set_flip(::flip flip) noexcept {
  auto& o = _registry.get<::orientation>(_e);
  o.flip = flip;

  auto& s = _registry.get<playback>(_e);
  s.redraw = true;
}

void entityproxy::set_onmail(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_e);
  c.on_mail = std::move(fn);
}

void entityproxy::set_onhover(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_e);
  c.on_hover = std::move(fn);
}

void entityproxy::set_onunhover(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_e);
  c.on_unhover = std::move(fn);
}

void entityproxy::set_ontouch(sol::protected_function fn) {
  auto& c = _registry.get<callbacks>(_e);
  c.on_touch = std::move(fn);
}

std::shared_ptr<entityproxy> entityproxy::clone() {
  auto e = _registry.create();

  if (const auto* m = _registry.try_get<metadata>(_e)) {
    _registry.emplace<metadata>(e, *m);
  }

  if (const auto* tn = _registry.try_get<tint>(_e)) {
    _registry.emplace<tint>(e, *tn);
  }

  if (const auto* sp = _registry.try_get<sprite>(_e)) {
    _registry.emplace<sprite>(e, *sp);
  }

  if (const auto* pb = _registry.try_get<playback>(_e)) {
    playback cpb = *pb;
    cpb.dirty = true;
    cpb.redraw = true;
    _registry.emplace<playback>(e, std::move(cpb));
  }

  if (const auto* tf = _registry.try_get<transform>(_e)) {
    _registry.emplace<transform>(e, *tf);
  }

  if (const auto* at = _registry.try_get<atlas>(_e)) {
    _registry.emplace<atlas>(e, *at);
  }

  if (const auto* ori = _registry.try_get<orientation>(_e)) {
    _registry.emplace<orientation>(e, *ori);
  }

  physics ph;
  ph.dirty = true;
  _registry.emplace<physics>(e, std::move(ph));

  if (const auto* rn = _registry.try_get<renderable>(_e)) {
    renderable crn = *rn;
    crn.z = rn->z + 1;
    _registry.emplace<renderable>(e, std::move(crn));
  }

  auto proxy = std::make_shared<entityproxy>(e, _registry);

  callbacks c;
  c.self = proxy;
  _registry.emplace<callbacks>(e, std::move(c));

  return proxy;
}
