#pragma once

#include "common.hpp"

#include "components.hpp"
#include "flip.hpp"
#include "kv.hpp"

class entityproxy {
public:
  entityproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~entityproxy() noexcept = default;

  uint64_t id() const noexcept;

  float x() const noexcept;
  void set_x(float x) noexcept;
  float y() const noexcept;
  void set_y(float y) noexcept;
  vec2 position() const noexcept;
  void set_position(const vec2& position) noexcept;

  uint8_t alpha() const noexcept;
  void set_alpha(uint8_t alpha) noexcept;
  double angle() const noexcept;
  void set_angle(double angle) noexcept;
  float scale() const noexcept;
  void set_scale(float scale) noexcept;

  ::flip flip() const noexcept;
  void set_flip(::flip flip) noexcept;

  bool visible() const noexcept;
  void set_visible(bool visible) noexcept;

  action_id action() const noexcept;
  void set_action(action_id id) noexcept;

  action_id kind() const noexcept;
  void set_kind(action_id id) noexcept;

  void set_onmail(sol::protected_function&& fn);
  void set_onhover(sol::protected_function&& fn);
  void set_onunhover(sol::protected_function&& fn);
  void set_ontouch(sol::protected_function&& fn);
  void set_onbegin(sol::protected_function&& fn);
  void set_onend(sol::protected_function&& fn);

  std::shared_ptr<entityproxy> clone();

  kv kv;
private:
  entt::entity _e;
  entt::registry& _registry;
};
