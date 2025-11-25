#pragma once

#include "common.hpp"

#include "geometry.hpp"

struct alignas(16) transform final {
  vec3 position{.0f, .0f, .0f};
  double angle{.0};
  float scale{1.f};
  bool dirty{true};
};

static_assert(sizeof(transform) == 32);
static_assert(alignof(transform) == 16);
static_assert(std::is_trivially_copyable_v<transform>);
static_assert(std::is_standard_layout_v<transform>);
