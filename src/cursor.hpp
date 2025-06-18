#pragma once

#include "common.hpp"

#include "event.hpp"
#include "eventreceiver.hpp"
#include "point.hpp"
#include "rectangle.hpp"
#include "resourcemanager.hpp"

namespace graphics {
constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";

struct keyframe final {
  geometry::rectangle frame;
  geometry::point offset;
  uint64_t duration{0};
};

struct animation final {
  bool oneshot{false};
  std::vector<keyframe> keyframes;
};

#ifdef EMSCRIPTEN
using animation_map = std::unordered_map<std::string, animation>;
#else
using animation_map = absl::flat_hash_map<std::string, animation>;
#endif

class cursor final : public input::eventreceiver {
public:
  explicit cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager);
  virtual ~cursor() noexcept = default;

  virtual void on_mouse_press(const input::event::mouse::button &event) override;
  virtual void on_mouse_release(const input::event::mouse::button &event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion &event) override;

  void update(float_t delta) noexcept;
  void draw() const noexcept;

  void handle(const std::string &message) noexcept;

private:
  geometry::point _position{0, 0};
  std::string _action{ACTION_DEFAULT};
  uint64_t _frame{0};
  uint64_t _last_frame{0};
  geometry::point _point;
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<pixmap> _spritesheet;
  animation_map _animations;
  std::optional<std::string> _queued_action;
};
}
