#include "entityproxy.hpp"
#include "components.hpp"
#include "helper.hpp"

entityproxy::entityproxy(entt::entity entity, entt::registry& registry) noexcept
  : _e(entity), _registry(registry) {
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
  t.position.x = y;
}

uint8_t entityproxy::alpha() const noexcept {
  const auto& t = _registry.get<tint>(_e);
  return t.a;
}

void entityproxy::set_alpha(uint8_t alpha) noexcept {
  auto& t = _registry.get<tint>(_e);
  t.a = alpha;
}

double entityproxy::angle() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.angle;
}

void entityproxy::set_angle(double angle) noexcept {
  auto& t = _registry.get<transform>(_e);
  t.angle = angle;
}

float entityproxy::scale() const noexcept {
  const auto& t = _registry.get<transform>(_e);
  return t.scale;
}

void entityproxy::set_scale(float scale) const noexcept {
  auto& t = _registry.get<transform>(_e);
  t.scale = scale;
}

std::optional<std::string> entityproxy::action() const noexcept {
  const auto& s = _registry.get<state>(_e);
  return s.action;
}

void entityproxy::set_action(std::optional<std::string_view> name) noexcept {
  auto& s = _registry.get<state>(_e);

  if (!name) {
    s.action = std::nullopt;
    s.dirty = true;
    return;
  }

  s.action = *name;
  s.dirty = true;
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
  callback.on_touch = interop::wrap_fn<void(float, float)>(std::move(fn));
}
