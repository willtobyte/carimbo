#pragma once

#include "common.hpp"

#include "kv.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"

namespace framework {
struct keyframe final {
  geometry::rectangle frame;
  geometry::point offset;
  uint64_t duration{0};
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::optional<geometry::rectangle> hitbox;
  std::shared_ptr<audio::soundfx> effect;
  std::vector<keyframe> keyframes;
};

#ifdef EMSCRIPTEN
using animation_map = std::unordered_map<std::string, animation>;
#else
using animation_map = absl::flat_hash_map<std::string, animation>;
#endif

class object final : public std::enable_shared_from_this<object> {
public:
  object();
  virtual ~object();

  uint64_t id() const;

  std::string kind() const;

  std::string scope() const;

  void update(float_t delta) noexcept;

  void draw() const noexcept;

  geometry::point position() const;
  float_t x() const;
  void set_x(float_t x);
  float_t y() const;
  void set_y(float_t y);

  void move(float_t x_velocity, float_t y_velocity);
  void set_velocity(const algebra::vector2d &velocity);
  algebra::vector2d velocity() const;

  void set_placement(float_t x, float_t y);
  geometry::point placement() const;

  void set_alpha(uint8_t alpha);
  uint8_t alpha() const;

  void set_scale(float_t scale);
  float_t scale() const;

  void set_onupdate(std::function<void(std::shared_ptr<object>)> fn);
  void set_onanimationfinished(std::function<void(std::shared_ptr<object>, const std::string &)> fn);
  void set_onmail(std::function<void(std::shared_ptr<object>, const std::string &)> fn);
  void set_ontouch(std::function<void(std::shared_ptr<object>, float_t, float_t)> fn);
  void set_onhover(std::function<void(std::shared_ptr<object>)> fn);
  void set_onunhover(std::function<void(std::shared_ptr<object>)> fn);
  void set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)> fn);
  void set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)> fn);

  void set_reflection(graphics::reflection reflection);
  graphics::reflection reflection() const;

  void set_action(const std::string &action);
  void unset_action();
  void hide();
  std::string action() const;

  bool intersects(std::shared_ptr<object> other) const;

  void on_email(const std::string &message);

  void on_touch(float_t x, float_t y);
  void on_motion(float_t x, float_t y);
  void on_hover();
  void on_unhover();

  memory::kv &kv();

private:
  friend class objectmanager;

  uint64_t _id;
  uint64_t _frame;
  uint64_t _last_frame;
  double_t _angle;
  uint8_t _alpha;
  float_t _scale;
  graphics::reflection _reflection;
  bool _hover;

  geometry::point _position;
  algebra::vector2d _velocity;
  std::string _kind;
  std::string _scope;
  std::string _action;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  animation_map _animations;

  memory::kv _kv;
  uint64_t _tick_count{0};
  uint64_t _last_tick{0};
  std::function<void(std::shared_ptr<object>, float_t, float_t)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>)> _onupdate;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onanimationfinished;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>> _collisionmapping;
  std::unordered_map<uint64_t, std::function<void(std::shared_ptr<object>)>> _tickinmapping;
};
}
