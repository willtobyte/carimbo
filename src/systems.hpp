#pragma once

#include "common.hpp"

class animationsystem final {
public:
  void update(entt::registry& registry, uint64_t now) noexcept;
};

class physicssystem final {
public:
  void update(entt::registry& registry, b2WorldId world, float delta) noexcept;
};

class renderablesystem final {
public:
  void draw(const entt::registry& registry) const noexcept;
};
