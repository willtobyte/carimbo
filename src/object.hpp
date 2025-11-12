#pragma once

#include "common.hpp"

#include "kv.hpp"

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

using animation_map = std::unordered_map<std::string, animation, string_hash, std::equal_to<>>;

class object final : public std::enable_shared_from_this<object> {
public:
  object();
  virtual ~object();
  std::string_view kind() const;
  std::string_view scope() const;

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

  void set_action(std::optional<std::string_view> action);
  std::string_view action() const;

  void set_onbegin(sol::protected_function fn);
  void set_onend(sol::protected_function fn);
  void set_onmail(sol::protected_function fn);
  void set_ontouch(sol::protected_function fn);
  void set_onhover(sol::protected_function fn);
  void set_onunhover(sol::protected_function fn);
  void set_oncollision(std::string_view kind, sol::protected_function fn);

  void on_email(std::string_view message);

  void on_touch(float x, float y);
  void on_hover();
  void on_unhover();

  memory::kv& kv() noexcept;

  uint64_t id() const noexcept;

protected:
  void suspend();

private:
  friend class objectmanager;
  friend class tilemap;
  friend class scene;
  friend class world;

  uint64_t _id;
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
  animation_map _animations;
  std::optional<std::reference_wrapper<animation>> _current_animation;

  bool _dirty{true};

  memory::kv _kv;
  std::function<void(std::shared_ptr<object>, float, float)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onbegin;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onend;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>, string_hash, std::equal_to<>> _collision_mapping;
};
}
