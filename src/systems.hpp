#pragma once

#include "common.hpp"
#include "components.hpp"

class animationsystem final {
public:
  explicit animationsystem(entt::registry& registry) noexcept
    : _entt(registry), _view(registry.view<std::shared_ptr<const atlas>, playback>()) {}

  void update(uint64_t now) noexcept;

private:
  using view_type = decltype(std::declval<entt::registry&>().view<std::shared_ptr<const atlas>, playback>());

  entt::registry& _entt;
  view_type _view;
};

class physicssystem final {
public:
  explicit physicssystem(entt::registry& registry) noexcept
    : _registry(registry), _group(registry.group<transform, rigidbody>(entt::get<playback, renderable>)) {}

  void update(b2WorldId world, float delta) noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<transform, rigidbody>(entt::get<playback, renderable>));

  entt::registry& _registry;
  group_type _group;
  float _accumulator{};
};

class rendersystem final {
public:
  explicit rendersystem(entt::registry& registry) noexcept
    : _registry(registry) {}

  void draw() const noexcept;

private:
  entt::registry& _registry;
};

class scriptsystem final {
public:
  explicit scriptsystem(entt::registry& registry) noexcept
    : _view(registry.view<scriptable>()) {}

  void update(float delta);

private:
  using view_type = decltype(std::declval<entt::registry&>().view<scriptable>());

  view_type _view;
};