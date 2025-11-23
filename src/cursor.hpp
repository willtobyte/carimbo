#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "vector.hpp"

namespace framework {
  class resourcemanager;
}

namespace graphics {
  class pixmap;
}

namespace graphics {
constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";

struct keyframe final {
  uint64_t duration{0};
  math::vec2 offset;
  math::vec4 frame;
};

struct animation final {
  bool oneshot{false};
  std::vector<keyframe> keyframes;
};

class cursor final : public input::eventreceiver {
public:
  explicit cursor(std::string_view name, std::shared_ptr<framework::resourcemanager> resourcemanager);
  virtual ~cursor() = default;

  virtual void on_mouse_release(const input::event::mouse::button& event) override;
  virtual void on_mouse_motion(const input::event::mouse::motion& event) override;

  void update(float delta);
  void draw() const;

  void handle(std::string_view message);

private:
  uint64_t _frame{0};
  uint64_t _last_frame{0};
  math::vec2 _position{0, 0};
  math::vec2 _point;
  std::string _action{ACTION_DEFAULT};
  std::shared_ptr<framework::resourcemanager> _resourcemanager;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  std::unordered_map<std::string, animation> _animations;
  std::optional<std::reference_wrapper<animation>> _current_animation;
  std::optional<std::string> _queued_action;
};
}
