#pragma once

#include "common.hpp"

#include "eventreceiver.hpp"
#include "geometry.hpp"

  class resourcemanager;

  class pixmap;

constexpr auto ACTION_DEFAULT = "default";
constexpr auto ACTION_LEFT = "left";
constexpr auto ACTION_RIGHT = "right";

struct keyframe final {
  uint64_t duration{0};
  vec2 offset;
  quad frame;

  friend void from_json(const nlohmann::json& j, keyframe& o) {
    o.frame = j["quad"].get<quad>();

    if (j.contains("offset")) {
      o.offset = j["offset"].get<vec2>();
    }

    o.duration = j["duration"].get<uint64_t>();
  }
};

struct bounds final {
  std::optional<uint8_t> type;
  std::bitset<256> reagents;
  quad rectangle;

  friend void from_json(const nlohmann::json& j, bounds& o) {
    if (j.contains("type")) {
      o.type = j["type"].get<uint16_t>();
    }

    o.rectangle = j["quad"].get<quad>();
  }
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::optional<bounds> bounds;
  std::shared_ptr<soundfx> effect;
  boost::container::small_vector<keyframe, 16> keyframes;
};

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
  boost::unordered_flat_map<std::string, animation, transparent_string_hash, std::equal_to<>> _animations;
  std::optional<std::reference_wrapper<animation>> _current_animation;
  std::optional<std::string> _queued_action;
};
