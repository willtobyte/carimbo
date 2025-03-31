#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "point.hpp"
#include "rect.hpp"
#include "reflection.hpp"
#include "resourcemanager.hpp"

namespace graphics {
namespace {
constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";
}

namespace graphics {
struct keyframe {
  geometry::rect frame;
  geometry::point offset;
  uint64_t duration{0};
};

struct animation {
  bool oneshot{false};
  std::vector<keyframe> keyframes;
};
}

class cursor : public input::eventreceiver {
public:
  explicit cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager);
  virtual ~cursor() = default;

  /* TODO object manager will also listen for some mouse events to trigger object action, here we just take care of animation */
  virtual void on_mousemotion(const input::mousemotionevent &event) noexcept;
  virtual void on_mousebuttondown(const input::mousebuttonevent &event) noexcept;
  virtual void on_mousebuttonup(const input::mousebuttonevent &event) noexcept;

  void update(float_t delta) noexcept;
  void draw() const noexcept;

  void handle(const std::string &message) noexcept;

private:
  geometry::point _position{0, 0};
  std::string _action{ACTION_DEFAULT};
  uint64_t _frame{0};
  uint64_t _last_frame{0};
  geometry::point _point{};
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<pixmap> _spritesheet;
  std::unordered_map<std::string, graphics::animation> _animations;
  std::optional<std::string> _queued_action;
};
}
