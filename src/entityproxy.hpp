#pragma once

#include "common.hpp"

#include "components.hpp"
#include "flip.hpp"
#include "kv.hpp"

struct ipcproxy;

class entityproxy {
  friend struct ipcproxy;
public:
  entityproxy(entt::entity entity, entt::registry& registry) noexcept;
  ~entityproxy() noexcept = default;

  [[nodiscard]] uint64_t id() const noexcept;

  [[nodiscard]] float x() const noexcept;
  void set_x(float x) noexcept;
  [[nodiscard]] float y() const noexcept;
  void set_y(float y) noexcept;
  [[nodiscard]] vec2 position() const noexcept;
  void set_position(const vec2& position) noexcept;

  [[nodiscard]] uint8_t alpha() const noexcept;
  void set_alpha(uint8_t value) noexcept;
  [[nodiscard]] double angle() const noexcept;
  void set_angle(double value) noexcept;
  [[nodiscard]] float scale() const noexcept;
  void set_scale(float value) noexcept;

  [[nodiscard]] ::flip flip() const noexcept;
  void set_flip(::flip value) noexcept;

  [[nodiscard]] bool visible() const noexcept;
  void set_visible(bool value) noexcept;

  [[nodiscard]] symbol action() const noexcept;
  void set_action(symbol value) noexcept;

  [[nodiscard]] symbol kind() const noexcept;
  void set_kind(symbol value) noexcept;

  void set_onmail(sol::protected_function fn);
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);
  void set_onbegin(sol::protected_function fn);
  void set_onend(sol::protected_function fn);
  void set_oncollision(sol::protected_function fn);
  void set_oncollisionend(sol::protected_function fn);

  [[nodiscard]] std::shared_ptr<entityproxy> clone();

  kv kv;

private:
  entt::entity _entity;
  entt::registry& _registry;
};
