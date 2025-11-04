#pragma once

#include "common.hpp"

#include "kv.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "pixmap.hpp"
#include "soundfx.hpp"

namespace framework {
class world;

struct pose final {
  float px, py, radians, hx, hy;
};

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
  object() noexcept;
  virtual ~object() noexcept;
  std::string kind() const noexcept;
  std::string scope() const noexcept;

  void update(float delta, uint64_t now) noexcept;

  void draw() const noexcept;

  geometry::point position() const noexcept;
  float x() const noexcept;
  void set_x(float x) noexcept;
  float y() const noexcept;
  void set_y(float y) noexcept;

  void set_placement(float x, float y) noexcept;
  geometry::point placement() const noexcept;

  void apply_velocity(float vx, float vy) noexcept;

  void set_alpha(uint8_t alpha) noexcept;
  uint8_t alpha() const noexcept;

  void set_scale(float scale) noexcept;
  float scale() const noexcept;

  void set_angle(double angle) noexcept;
  double angle() const noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;
  graphics::reflection reflection() const noexcept;

  bool visible() const noexcept;
  void set_visible(bool value) noexcept;

  void set_action(const std::optional<std::string>& action) noexcept;
  std::string action() const noexcept;

  void set_onbegin(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept;
  void set_onend(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept;
  void set_onmail(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept;
  void set_ontouch(std::function<void(std::shared_ptr<object>, float, float)>&& fn) noexcept;
  void set_onhover(std::function<void(std::shared_ptr<object>)>&& fn) noexcept;
  void set_onunhover(std::function<void(std::shared_ptr<object>)>&& fn) noexcept;
  void set_oncollision(const std::string& kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>&& fn) noexcept;

  void on_email(const std::string& message) noexcept;

  void on_touch(float x, float y) noexcept;
  void on_hover() noexcept;
  void on_unhover() noexcept;

  memory::kv& kv() noexcept;

  uint64_t id() const noexcept;

protected:
  std::optional<pose> compute_pose() const noexcept;
  void sync_body() noexcept;

private:
  friend class objectmanager;
  friend class tilemap;
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

  float _last_synced_scale{-1.0f};
  float _last_synced_hx{0.0f};
  float _last_synced_hy{0.0f};

  memory::kv _kv;
  std::function<void(std::shared_ptr<object>, float, float)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onbegin;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onend;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>> _collision_mapping;
};
}
