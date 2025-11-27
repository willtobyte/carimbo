#pragma once

#include "common.hpp"
#include "components.hpp"

class entityproxy {
public:
  entityproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~entityproxy() noexcept = default;

  std::string_view action() const noexcept;
  void set_action(std::string_view name) noexcept;
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);

private:
  entt::entity _entity;
  entt::registry& _registry;
};