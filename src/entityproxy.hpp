#pragma once

#include "common.hpp"

#include "kv.hpp"

class entityproxy {
public:
  entityproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~entityproxy() noexcept = default;

  std::optional<std::string> action() const noexcept;
  void set_action(std::optional<std::string_view> name) noexcept;
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);

  kv kv;
private:
  entt::entity _entity;
  entt::registry& _registry;
};
