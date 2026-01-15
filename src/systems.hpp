#pragma once

#include "common.hpp"
#include "components.hpp"
#include "physics.hpp"

class animationsystem final {
public:
  explicit animationsystem(entt::registry& registry) noexcept
    : _entt(registry), _view(registry.view<std::shared_ptr<const atlas>, playback>()) {}

  void update(uint64_t now);

private:
  using view_type = decltype(std::declval<entt::registry&>().view<std::shared_ptr<const atlas>, playback>());

  entt::registry& _entt;
  view_type _view;
};

class physicssystem final {
public:
  explicit physicssystem(entt::registry& registry, physics::world& world) noexcept
    : _registry(registry), _world(world), _group(registry.group<transform, physics::body>(entt::get<playback, renderable>)) {}

  void update(float delta);

private:
  using group_type = decltype(std::declval<entt::registry&>().group<transform, physics::body>(entt::get<playback, renderable>));

  entt::registry& _registry;
  physics::world& _world;
  group_type _group;
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

class velocitysystem final {
public:
  explicit velocitysystem(entt::registry& registry) noexcept
    : _view(registry.view<transform, velocity>()) {}

  void update(float delta);

private:
  using view_type = decltype(std::declval<entt::registry&>().view<transform, velocity>());

  view_type _view;
};