#pragma once

#include "common.hpp"

#include "kv.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "vector2d.hpp"
#include "pixmap.hpp"
#include "soundfx.hpp"

namespace framework {
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

  uint64_t id() const noexcept;

  std::string kind() const noexcept;

  std::string scope() const noexcept;

  void update(float delta) noexcept;

  void draw() const noexcept;

  geometry::point position() const noexcept;
  float x() const noexcept;
  void set_x(float x) noexcept;
  float y() const noexcept;
  void set_y(float y) noexcept;

  void set_velocity(const algebra::vector2d& velocity) noexcept;
  algebra::vector2d& velocity() noexcept;

  void set_placement(float x, float y) noexcept;
  geometry::point placement() const noexcept;

  void set_alpha(uint8_t alpha) noexcept;
  uint8_t alpha() const noexcept;

  void set_scale(float scale) noexcept;
  float scale() const noexcept;

  void set_angle(double angle) noexcept;
  double angle() const noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;
  graphics::reflection reflection() const noexcept;

  void set_visible(bool value) noexcept;
  bool visible() const noexcept;

  void set_action(const std::optional<std::string>& action) noexcept;
  void unset_action() noexcept;
  std::string action() const noexcept;

  std::optional<geometry::rectangle> boundingbox() const noexcept;

  void set_onupdate(std::function<void(std::shared_ptr<object>)>&& fn);
  void set_onbegin(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn);
  void set_onend(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn);
  void set_onmail(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn);
  void set_ontouch(std::function<void(std::shared_ptr<object>, float, float)>&& fn);
  void set_onhover(std::function<void(std::shared_ptr<object>)>&& fn);
  void set_onunhover(std::function<void(std::shared_ptr<object>)>&& fn);
  void set_oncollision(const std::string& kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>&& fn);
  void set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)>&& fn);

  void on_email(const std::string& message);

  void on_touch(float x, float y);
  void on_hover();
  void on_unhover();

  bool dirty() noexcept;

  memory::kv& kv();

protected:
    void move(float delta) noexcept;

private:
  friend class objectmanager;
  friend class tilemap;
  friend class world;

  uint64_t _id;
  bool _visible;
  std::size_t _frame;
  uint64_t _last_frame;
  double _angle;
  uint8_t _alpha;
  float _scale;
  graphics::reflection _reflection;
  bool _hover{false};
  mutable bool _dirty{true};

  geometry::point _position;
  algebra::vector2d _velocity;
  std::string _kind;
  std::string _scope;
  std::string _action;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  std::unordered_map<std::string, animation> _animations;
  mutable std::optional<geometry::rectangle> _boundingbox{};

  memory::kv _kv;
  std::function<void(std::shared_ptr<object>, float, float)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>)> _onupdate;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onbegin;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onend;
  std::function<void(std::shared_ptr<object>, const std::string& )> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>> _collisionmapping;
};
}
