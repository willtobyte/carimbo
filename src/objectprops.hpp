#pragma once

#include "animation.hpp"
#include "common.hpp"
#include "pixmap.hpp"
#include "point.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"

namespace framework {

struct objectprops {
  uint64_t id{};
  uint32_t frame{};
  uint32_t last_frame{};
  double_t angle{};
  uint8_t alpha{255};
  geometry::point position{};
  float_t scale{1.f};
  algebra::vector2d velocity{};
  std::string kind{};
  std::string action{};
  graphics::reflection reflection{graphics::reflection::none};
  std::shared_ptr<graphics::pixmap> spritesheet{};
  std::unordered_map<std::string, graphics::animation> animations{};
};
}
