#pragma once

#include "common.hpp"

struct particlebatch;
class particlepool;

enum class drawablekind : uint8_t {
  entity,
  batch
};

struct drawslice final {
  int16_t z;
  drawablekind kind;
  union {
    entt::entity entity;
    particlebatch* batch;
  };
};

class renderqueue final {
public:
  renderqueue(const entt::registry& registry, const particlepool& particlepool) noexcept;
  ~renderqueue() noexcept = default;

  void update() noexcept;
  void draw() const noexcept;

private:
  const entt::registry& _registry;
  const particlepool& _particlepool;
  boost::container::small_vector<drawslice, 128> _drawables;
};
