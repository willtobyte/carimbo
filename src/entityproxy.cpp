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

std::optional<std::string> entityproxy::action() const noexcept {
  const auto& s = _registry.get<playback>(_e);
  return s.action;
}

void entityproxy::set_action(std::optional<std::string_view> name) noexcept {
  auto& s = _registry.get<playback>(_e);

  if (!name) {
    s.action = std::nullopt;
    s.current_frame = 0;
    s.dirty = true;
    s.cache = nullptr;
    return;
  }

  s.action = *name;
  s.current_frame = 0;
  s.dirty = true;
  s.cache = nullptr;
}

void entityproxy::set_onmail(sol::protected_function fn) {
  auto& callback = _registry.get<callbacks>(_e);
  callback.on_mail = interop::wrap_fn<void(std::shared_ptr<entityproxy>, std::string_view)>(std::move(fn));
}

void entityproxy::set_onhover(sol::protected_function fn) {
  auto& callback = _registry.get<callbacks>(_e);
  callback.on_hover = interop::wrap_fn<void(std::shared_ptr<entityproxy>)>(std::move(fn));
}

void entityproxy::set_onunhover(sol::protected_function fn) {
  auto& callback = _registry.get<callbacks>(_e);
  callback.on_unhover = interop::wrap_fn<void(std::shared_ptr<entityproxy>)>(std::move(fn));
}

void entityproxy::set_ontouch(sol::protected_function fn) {
  auto& callback = _registry.get<callbacks>(_e);
  callback.on_touch = interop::wrap_fn<void(std::shared_ptr<entityproxy>, float, float)>(std::move(fn));
}

std::string_view entityproxy::kind() const noexcept {
  const auto& m = _registry.get<metadata>(_e);
  return m.kind;
}

void entityproxy::set_kind(std::string_view kind) noexcept {
  auto& m = _registry.get<metadata>(_e);
  m.kind = kind;
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
