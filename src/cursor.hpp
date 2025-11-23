#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "vector.hpp"
#include "object.hpp"

  class resourcemanager;

  class pixmap;

constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";

class cursor final : public eventreceiver {
public:
  explicit cursor(std::string_view name, std::shared_ptr<resourcemanager> resourcemanager);
  virtual ~cursor() = default;

  virtual void on_mouse_release(const event::mouse::button& event) override;
  virtual void on_mouse_motion(const event::mouse::motion& event) override;

  void update(float delta);
  void draw() const;

  void handle(std::string_view message);

private:
  uint64_t _frame{0};
  uint64_t _last_frame{0};
  vec2 _position{0, 0};
  vec2 _point;
  std::string _action{ACTION_DEFAULT};
  std::shared_ptr<resourcemanager> _resourcemanager;
  std::shared_ptr<pixmap> _spritesheet;
  std::unordered_map<std::string, animation> _animations;
  std::optional<std::reference_wrapper<animation>> _current_animation;
  std::optional<std::string> _queued_action;
};
