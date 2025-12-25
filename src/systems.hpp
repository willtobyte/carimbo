#pragma once

#include "common.hpp"
#include "components.hpp"

class animationsystem final {
public:
  explicit animationsystem(entt::registry& registry) noexcept
    : _group(registry.group<std::shared_ptr<const atlas>, playback, callbacks>()) {}

  void update(uint64_t now) noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<std::shared_ptr<const atlas>, playback, callbacks>());

  group_type _group;
};

class physicssystem final {
public:
  explicit physicssystem(entt::registry& registry) noexcept
    : _group(registry.group<transform, physics>(entt::get<playback, renderable>)) {}

  void update(b2WorldId world, float delta) noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<transform, physics>(entt::get<playback, renderable>));

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

  void update(float delta) noexcept;

private:
  using view_type = decltype(std::declval<entt::registry&>().view<scriptable>());

  view_type _view;
};