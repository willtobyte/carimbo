#pragma once

#include "common.hpp"

#include "point.hpp"
#include "rectangle.hpp"
#include "vector2d.hpp"

namespace graphics {
struct keyframe final {
  geometry::rectangle frame;
  geometry::point offset;
  uint64_t duration{0};
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::optional<geometry::rectangle> hitbox;
  std::vector<keyframe> keyframes;
};
}

namespace framework {
struct objectprops final {
  uint64_t id{};
  uint64_t frame{};
  uint64_t last_frame{};
  double_t angle{};
  uint8_t alpha{255};
  geometry::point position{};
  float_t scale{1.f};
  algebra::vector2d velocity{};
  std::string kind{};
  std::string action{};
  bool hover{false};
  graphics::reflection reflection{graphics::reflection::none};
  std::shared_ptr<graphics::pixmap> spritesheet{};
  std::unordered_map<std::string, graphics::animation> animations{};
};
}
