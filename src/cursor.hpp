#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "geometry.hpp"

constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";

struct keyframe final {
  uint64_t duration{0};
  vec2 offset;
  quad frame;

  keyframe() noexcept = default;

  explicit keyframe(unmarshal::json node) noexcept
      : duration(node["duration"].get<uint64_t>()),
        offset(node["offset"].get<vec2>(vec2{})),
        frame(node["quad"].get<quad>()) {}
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::shared_ptr<soundfx> effect;
  boost::container::small_vector<keyframe, 16> keyframes;
};

class cursor final : public eventreceiver {
public:
  explicit cursor(std::string_view name);
  virtual ~cursor();

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
  boost::static_string<48> _action{ACTION_DEFAULT};
  std::shared_ptr<pixmap> _spritesheet;
  boost::unordered_flat_map<std::string, animation, transparent_string_hash, std::equal_to<>> _animations;
  animation* _current_animation{nullptr};
  boost::static_string<48> _queued_action;
};
