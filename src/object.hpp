#pragma once

#include "common.hpp"

#include "kv.hpp"

namespace framework {
class world;

struct keyframe final {
  uint64_t duration{0};
  math::vec2 offset;
  math::vec4 frame;

  friend void from_json(const nlohmann::json& j, keyframe& o);
};

struct bounds final {
  std::optional<uint8_t> type;
  std::bitset<256> reagents;
  math::vec4 rectangle;

  friend void from_json(const nlohmann::json& j, framework::bounds& o);
};

struct animation final {
  bool oneshot{false};
  std::optional<std::string> next;
  std::optional<framework::bounds> bounds;
  std::shared_ptr<audio::soundfx> effect;
  std::vector<keyframe> keyframes;
};

using animation_map = std::unordered_map<std::string, animation, string_hash, string_equal>;

class object final : public std::enable_shared_from_this<object> {
public:
  object();
  ~object();

  std::string_view kind() const noexcept;

  void update(float delta, uint64_t now);

  void draw() const;

  math::vec2 position() const noexcept;
  float x() const noexcept;
  void set_x(float x) noexcept;
  float y() const noexcept;
  void set_y(float y) noexcept;

  void set_placement(float x, float y) noexcept;
  math::vec2 placement() const noexcept;

  void set_alpha(uint8_t alpha) noexcept;
  uint8_t alpha() const noexcept;

  void set_scale(float scale) noexcept;
  float scale() const noexcept;

  void set_angle(double angle) noexcept;
  double angle() const noexcept;

  void set_reflection(graphics::reflection reflection) noexcept;
  graphics::reflection reflection() const noexcept;

  bool visible() const noexcept;
  void set_visible(bool value);

  void set_action(std::optional<std::string_view> action);
  std::string_view action() const noexcept;

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

  struct controller final {
    animation& _animation;
    std::size_t _frame{0};
    uint64_t _last_tick{0};

    math::vec2 _offset;
    math::vec4 _source;
    math::vec4 _bounds;
    bool _has_keyframe{false};
    bool _has_bounds{false};

    explicit controller(animation& animation);
    void frooze();
    bool tick(uint64_t now);
    void reset();
    bool finished() const noexcept;
    bool valid() const noexcept;
    const math::vec4& bounds() const noexcept;
  };

  struct body final {
    b2BodyId _id{b2_nullBodyId};
    b2ShapeId _shape{b2_nullShapeId};
    bool _enabled{false};

    struct {
      math::vec2 position;
      math::vec4 bounds;
      float scale{0.0f};
      double angle{0.0};
      bool valid{false};
    } _last_sync;

    std::weak_ptr<world> _world;

    body() = default;
    body(std::weak_ptr<world> world);
    ~body();
    void enable();
    void disable();
    bool missing() const noexcept;
    bool valid() const noexcept;
    void sync(const math::vec4& bounds, const math::vec2& position, float scale, double angle, uint64_t id);
  };

  math::vec2 _position;
  double _angle;
  float _scale;
  uint8_t _alpha;
  graphics::reflection _reflection;
  bool _visible{true};
  bool _dirty{true};
  mutable bool _redraw{true};
  mutable math::vec4 _destination;

  std::string _action;
  std::optional<controller> _animation;
  body _body;

  uint64_t _id;
  std::shared_ptr<graphics::pixmap> _spritesheet;
  animation_map _animations;
  std::string _kind;

  std::function<void(std::shared_ptr<object>, float, float)> _ontouch;
  std::function<void(std::shared_ptr<object>)> _onhover;
  std::function<void(std::shared_ptr<object>)> _onunhover;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onbegin;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onend;
  std::function<void(std::shared_ptr<object>, std::string_view)> _onmail;
  std::unordered_map<std::string, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>, string_hash, string_equal> _collision_mapping;
  memory::kv _kv;
};
}
