#pragma once

#include "common.hpp"

#include "kv.hpp"

class entityproxy {
public:
  entityproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~entityproxy() noexcept = default;

  float x() const noexcept;
  void set_x(float x) noexcept;
  float y() const noexcept;
  void set_y(float y) noexcept;

  uint8_t alpha() const noexcept;
  void set_alpha(uint8_t alpha) noexcept;
  double angle() const noexcept;
  void set_angle(double angle) noexcept;
  float scale() const noexcept;
  void set_scale(float scale) noexcept;

  std::optional<std::string> action() const noexcept;
  void set_action(std::optional<std::string_view> name) noexcept;
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);

  std::string_view kind() const noexcept;
  void set_kind(std::string_view kind) noexcept;

  void set_placement(float x, float y) noexcept;

  kv kv;
private:
  entt::entity _e;
  entt::registry& _registry;
};
