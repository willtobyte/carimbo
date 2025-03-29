#pragma once

#include "common.hpp"

#include "point.hpp"
#include "rect.hpp"

namespace graphics {
struct keyframe {
  geometry::rect frame;
  geometry::point offset;
  uint64_t duration{0};
};

struct animation {
  std::optional<std::string> next;
  bool oneshot{false};
  std::optional<geometry::rect> hitbox;
  std::vector<keyframe> keyframes;
};
}
