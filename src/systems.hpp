#pragma once

#include "common.hpp"
#include "components.hpp"

class animationsystem final {
public:
  explicit animationsystem(entt::registry& registry) noexcept
    : _group(registry.group<atlas, playback, callbacks>()) {}

  void update(uint64_t now) noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<atlas, playback, callbacks>());

  group_type _group;
};

class physicssystem final {
public:
  explicit physicssystem(entt::registry& registry) noexcept
    : _group(registry.group<transform, playback, physics, renderable>()) {}

  void update(b2WorldId world, float delta) noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<transform, playback, physics, renderable>());

  group_type _group;
};

class rendersystem final {
public:
  explicit rendersystem(entt::registry& registry) noexcept
    : _group(registry.group<renderable>(entt::get<transform, tint, sprite, playback, orientation>)) {}

  void draw() const noexcept;

private:
  using group_type = decltype(std::declval<entt::registry&>().group<renderable>(entt::get<transform, tint, sprite, playback, orientation>));

  group_type _group;
};
