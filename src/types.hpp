#pragma once

#include "common.hpp"

#include "point.hpp"
#include "rect.hpp"

namespace graphics {
struct keyframe {
  geometry::rect frame;
  geometry::point offset;
  uint64_t duration{};
  bool singleshoot{};
};

struct animation {
  std::optional<geometry::rect> hitbox;
  std::vector<keyframe> keyframes;
};
}
