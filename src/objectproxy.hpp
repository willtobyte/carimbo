#pragma once

#include "common.hpp"

#include "components.hpp"
#include "flip.hpp"
#include "kv.hpp"

struct metaobject;

class objectproxy {
  friend struct metaobject;

public:
  objectproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~objectproxy() noexcept = default;

  [[nodiscard]] uint64_t id() const noexcept;

  [[nodiscard]] float x() const noexcept;
  void set_x(float x) noexcept;
  [[nodiscard]] float y() const noexcept;
  void set_y(float y) noexcept;
  [[nodiscard]] vec2 position() const noexcept;
  void set_position(const vec2& position) noexcept;

  [[nodiscard]] vec2 velocity() const noexcept;
  void set_velocity(const vec2& velocity) noexcept;

  [[nodiscard]] uint8_t alpha() const noexcept;
  void set_alpha(uint8_t value) noexcept;
  [[nodiscard]] double angle() const noexcept;
  void set_angle(double value) noexcept;
  [[nodiscard]] float scale() const noexcept;
  void set_scale(float value) noexcept;

  [[nodiscard]] ::flip flip() const noexcept;
  void set_flip(::flip value) noexcept;

  [[nodiscard]] int z() const noexcept;
  void set_z(int value) noexcept;

  [[nodiscard]] bool visible() const noexcept;
  void set_visible(bool value) noexcept;

  [[nodiscard]] std::string_view action() const noexcept;
  void set_action(std::string_view value);

  [[nodiscard]] std::string_view kind() const noexcept;
  void set_kind(std::string_view value);

  [[nodiscard]] std::string_view name() const noexcept;

  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);
  void set_onbegin(sol::protected_function fn);
  void set_onend(sol::protected_function fn);
  void set_oncollision(sol::protected_function fn);
  void set_oncollisionend(sol::protected_function fn);
  void set_ontick(sol::protected_function fn);

  [[nodiscard]] std::shared_ptr<objectproxy> clone();

  [[nodiscard]] bool alive() const noexcept;
  void die() noexcept;

  kv kv;

private:
  entt::entity _entity;
  entt::registry& _registry;
};
