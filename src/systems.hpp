#pragma once

#include "common.hpp"
#include "components.hpp"
#include "physics.hpp"

class animationsystem final {
public:
  explicit animationsystem(entt::registry& registry) noexcept
    : _entt(registry), _view(registry.view<const atlas*, playback, dirtable>()) {}

  void update(uint64_t now);

private:
  using view_type = decltype(std::declval<entt::registry&>().view<const atlas*, playback, dirtable>());

  entt::registry& _entt;
  view_type _view;
};

class physicssystem final {
public:
  explicit physicssystem(entt::registry& registry, physics::world& world) noexcept
    : _registry(registry), _world(world), _group(registry.group<transform, physics::body>(entt::get<playback, renderable, tint, dirtable>)) {}

  void update(float delta);

private:
  using group_type = decltype(std::declval<entt::registry&>().group<transform, physics::body>(entt::get<playback, renderable, tint, dirtable>));

  entt::registry& _registry;
  physics::world& _world;
  group_type _group;
};

class rendersystem final {
public:
  explicit rendersystem(entt::registry& registry) noexcept
    : _registry(registry),
      _view(registry.view<renderable, transform, tint, sprite, playback, orientation, dirtable, drawable>()) {}

  void update() noexcept;
  void draw() const noexcept;

private:
  using view_type = decltype(std::declval<entt::registry&>()
    .view<renderable, transform, tint, sprite, playback, orientation, dirtable, drawable>());

  entt::registry& _registry;
  view_type _view;
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
    : _view(registry.view<transform, velocity, dirtable>()) {}

  void update(float delta);

private:
  using view_type = decltype(std::declval<entt::registry&>().view<transform, velocity, dirtable>());

  view_type _view;
};