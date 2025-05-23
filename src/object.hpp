#pragma once

#include "common.hpp"

#include "kv.hpp"
#include "objectprops.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"

namespace framework {
class object : public std::enable_shared_from_this<object> {
public:
  explicit object(const objectprops &props);
  virtual ~object();

  uint64_t id() const;

  std::string kind() const;

  void update(float_t delta);

  void draw() const;

  objectprops &props();
  const objectprops &props() const;
  void set_props(const objectprops &props);

  void hide();

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
  void set_onmotion(std::function<void(std::shared_ptr<object>, float_t, float_t)> fn);
  void set_onhover(std::function<void(std::shared_ptr<object>)> fn);
  void set_onunhover(std::function<void(std::shared_ptr<object>)> fn);
  void set_onkeypress(std::function<void(std::shared_ptr<object>, int32_t)> fn);
  void set_onkeyrelease(std::function<void(std::shared_ptr<object>, int32_t)> fn);

  void set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)> fn);
  void set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)> fn);

  void set_reflection(graphics::reflection reflection);
  graphics::reflection reflection() const;

  void set_action(const std::string &action);
  void unset_action();
  std::string action() const;

  bool intersects(std::shared_ptr<object> other) const;

  void on_email(const std::string &message);

  void on_key_press(int32_t key);
  void on_key_release(int32_t key);
  void on_touch(float_t x, float_t y);
  void on_motion(float_t x, float_t y);
  void on_hover();
  void on_unhover();

  memory::kv &kv();

private:
  friend class objectmanager;

  memory::kv _kv;
  objectprops _props;
  uint64_t _tick_count{0};
  uint64_t _last_tick{0};
  std::function<void(std::shared_ptr<object>, int32_t)> _onkeypress;
  std::function<void(std::shared_ptr<object>, int32_t)> _onkeyrelease;
  std::function<void(std::shared_ptr<object>, float_t, float_t)> _ontouch;
  std::function<void(std::shared_ptr<object>, float_t, float_t)> _onmotion;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>)> _onupdate;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onanimationfinished;
  std::function<void(std::shared_ptr<object>, const std::string &)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>> _collisionmapping;
  std::unordered_map<uint64_t, std::function<void(std::shared_ptr<object>)>> _tickinmapping;
};
}
