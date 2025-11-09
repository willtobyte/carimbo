#pragma once

#include "common.hpp"

#include "kv.hpp"
#include "physics.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "pixmap.hpp"
#include "soundfx.hpp"

namespace framework {
class world;

struct keyframe final {
  geometry::rectangle frame;
  geometry::point offset;
  uint64_t duration{0};

  friend void from_json(const nlohmann::json& j, keyframe& o);
};

struct bounds final {
  std::optional<uint8_t> type;
  std::bitset<256> reagents;
  geometry::rectangle rectangle;

  friend void from_json(const nlohmann::json& j, framework::bounds& o);
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::optional<framework::bounds> bounds;
  std::shared_ptr<audio::soundfx> effect;
  std::vector<keyframe> keyframes;
};

class object final : public std::enable_shared_from_this<object> {
public:
  object();
  virtual ~object();
  std::string kind() const;
  std::string scope() const;

  void update(float delta, uint64_t now);

  void draw() const;

  geometry::point position() const;
  float x() const;
  void set_x(float x);
  float y() const;
  void set_y(float y);

  void set_placement(float x, float y);
  geometry::point placement() const;

  void set_alpha(uint8_t alpha);
  uint8_t alpha() const;

  void set_scale(float scale);
  float scale() const;

  void set_angle(double angle);
  double angle() const;

  void set_reflection(graphics::reflection reflection);
  graphics::reflection reflection() const;

  bool visible() const;
  void set_visible(bool value);

  void set_action(const std::optional<std::string>& action);
  std::string action() const;

  void set_onbegin(sol::protected_function fn);
  void set_onend(sol::protected_function fn);
  void set_onmail(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_oncollision(std::string kind, sol::protected_function fn);

  void on_email(const std::string& message);

  void on_touch(float x, float y);
  void on_hover();
  void on_unhover();

  memory::kv& kv();

  uint64_t id() const;

protected:
  void suspend();

private:
  friend class objectmanager;
  friend class tilemap;
  friend class scene;
  friend class world;

  b2BodyId _body;
  b2ShapeId _collision_shape;
  std::weak_ptr<world> _world;

  std::size_t _frame;
  uint64_t _last_frame;
  double _angle;
  uint8_t _alpha;
  float _scale;
  graphics::reflection _reflection;
  bool _visible{true};

  geometry::point _position;
  std::string _kind;
  std::string _scope;
  std::string _action;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  std::unordered_map<std::string, animation> _animations;

  std::optional<physics::body_transform> _last_synced_transform;
  bool _need_update_physics{false};

  memory::kv _kv;
  std::function<void(std::shared_ptr<object>, float, float)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onbegin;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onend;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>, std::hash<std::string_view>, std::equal_to<>> _collision_mapping;
};
}
