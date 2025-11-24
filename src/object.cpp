#include "object.hpp"

#include "constant.hpp"
#include "kv.hpp"
#include "physics.hpp"
#include "reflection.hpp"
#include "world.hpp"

object::controller::controller(animation& animation) : _animation(animation), _last_tick(0) {
  frooze();
}

void object::controller::frooze() {
  const auto& keyframes = _animation.keyframes;

  _has_keyframe = _frame < keyframes.size();
  if (_has_keyframe) [[likely]] {
    const auto& keyframe = keyframes[_frame];
    _source = keyframe.frame;
    _offset = keyframe.offset;
  }

  _has_bounds = _animation.bounds.has_value();
  if (_has_bounds) [[likely]] {
    _bounds = _animation.bounds->rectangle;
  }
}

bool object::controller::tick(uint64_t now) {
  if (!_has_keyframe) [[unlikely]] return false;

  const auto& keyframes = _animation.keyframes;
  if (_frame >= keyframes.size() || keyframes[_frame].duration <= 0) [[unlikely]] return false;

  if (_last_tick == 0) [[unlikely]] {
    _last_tick = now;
    return false;
  }

  if (now - _last_tick < keyframes[_frame].duration) [[likely]] return false;

  ++_frame;
  _last_tick = now;
  frooze();
  return true;
}

void object::controller::reset() {
  _frame = 0;
  _last_tick = 0;
  frooze();
}

bool object::controller::finished() const noexcept {
  return _frame >= _animation.keyframes.size();
}

bool object::controller::valid() const noexcept {
  return _has_keyframe;
}

const vec4& object::controller::bounds() const noexcept {
  return _bounds;
}

object::body::~body() {
  if (b2Shape_IsValid(_shape)) [[likely]] {
    b2DestroyShape(_shape, false);
    _shape = b2_nullShapeId;
  }

  if (b2Body_IsValid(_id)) [[likely]] {
    b2DestroyBody(_id);
    _id = b2_nullBodyId;
  }

  _enabled = false;
}

object::body::body(std::weak_ptr<world> world) : _world(std::move(world)) {}

bool object::body::missing() const noexcept {
  return !b2Body_IsValid(_id);
}

bool object::body::valid() const noexcept {
  return b2Body_IsValid(_id);
}

void object::body::enable() {
  if (!b2Body_IsValid(_id) || _enabled) [[unlikely]] return;

  b2Body_Enable(_id);
  _enabled = true;
}

void object::body::disable() {
  if (!b2Body_IsValid(_id) || !_enabled) [[unlikely]] return;

  b2Body_Disable(_id);
  _enabled = false;
}

void object::body::sync(const vec4& bounds, const vec2& position, float scale, double angle, uint64_t id) {
  if (_last_sync.valid &&
      _last_sync.position == position &&
      _last_sync.bounds == bounds &&
      _last_sync.scale == scale &&
      _last_sync.angle == angle) [[likely]] {
    return;
  }

  const auto sw = bounds.w * scale;
  const auto sh = bounds.h * scale;
  const auto hx = 0.5f * sw;
  const auto hy = 0.5f * sh;

  const auto center_x = bounds.w * 0.5f;
  const auto center_y = bounds.h * 0.5f;
  const auto px = position.x + bounds.x + center_x;
  const auto py = position.y + bounds.y + center_y;
  const auto radians = static_cast<float>(angle) * DEGREES_TO_RADIANS;

  const auto box = b2MakeBox(hx, hy);
  const auto rotation = b2MakeRot(radians);
  const auto b2_position = b2Vec2{px, py};

  if (missing()) [[unlikely]] {
    auto world = _world.lock();
    if (!world) [[unlikely]] return;

    auto def = b2DefaultBodyDef();
    def.type = b2_kinematicBody;
    def.userData = id_to_userdata(id);
    def.position = b2_position;
    def.rotation = rotation;
    _id = b2CreateBody(*world, &def);

    auto sd = b2DefaultShapeDef();
    sd.isSensor = true;
    sd.enableSensorEvents = true;
    sd.filter.categoryBits = collisioncategory::player;
    sd.filter.maskBits = collisioncategory::player;
    _shape = b2CreatePolygonShape(_id, &sd, &box);
    _enabled = true;

    _last_sync.position = position;
    _last_sync.bounds = bounds;
    _last_sync.scale = scale;
    _last_sync.angle = angle;
    _last_sync.valid = true;
    return;
  }

  enable();
  b2Body_SetTransform(_id, b2_position, rotation);
  if (b2Shape_IsValid(_shape)) [[likely]] {
    b2Shape_SetPolygon(_shape, &box);
  }

  _last_sync.position = position;
  _last_sync.bounds = bounds;
  _last_sync.scale = scale;
  _last_sync.angle = angle;
  _last_sync.valid = true;
}

object::object()
  : _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(reflection::none) {
  _collision_mapping.reserve(8);
}

object::~object() {
  std::println("[object] destroyed {} {}", kind(), id());
}

std::string_view object::kind() const noexcept {
  return _kind;
}

vec2 object::position() const noexcept {
  return _position;
}

float object::x() const noexcept {
  return _position.x;
}

float object::y() const noexcept {
  return _position.y;
}

vec2 object::placement() const noexcept {
  return _position;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_x(float x) noexcept {
  if (_position.x == x) [[unlikely]] return;

  _position.x = x;
  _dirty = true;
  _redraw = true;
}

void object::set_y(float y) noexcept {
  if (_position.y == y) [[unlikely]] return;

  _position.y = y;
  _dirty = true;
  _redraw = true;
}

void object::set_placement(float x, float y) noexcept {
  if (_position.x == x && _position.y == y) [[unlikely]] return;

  _position.x = x;
  _position.y = y;
  _dirty = true;
  _redraw = true;
}

void object::update(float delta, uint64_t now) {
  if (!_animation) [[unlikely]] return;

  const auto changed = _animation->tick(now);

  if (!changed) [[likely]] {
    if (_animation->_has_bounds && _visible) [[likely]] {
      if (_body.valid() && !_dirty) [[likely]] {
        _body.enable();
        return;
      }

      _body.sync(_animation->bounds(), _position, _scale, _angle, _id);
      _dirty = false;
      return;
    }

    _body.disable();
    return;
  }

  _redraw = true;

  if (!_animation->finished()) [[likely]] {
    if (_animation->_has_bounds && _visible) [[likely]] {
      _body.sync(_animation->bounds(), _position, _scale, _angle, _id);
      _dirty = false;

      return;
    }

    _body.disable();

    return;
  }

  if (!_animation->_animation.oneshot) [[unlikely]] {
    _animation->reset();

    return;
  }

  if (auto fn = _onend; fn) {
    fn(shared_from_this(), _action);
  }

  if (_animation->_animation.next) [[likely]] {
    set_action(*_animation->_animation.next);
  }
}

void object::draw() const {
  if (!_visible || !_animation || !_animation->valid()) [[unlikely]] return;

  const auto& source = _animation->_source;
  const auto& offset = _animation->_offset;

  if (_redraw) [[unlikely]] {
    _destination = vec4{_position + offset, {source.w, source.h}};

    if (_scale != 1.0f) [[unlikely]] {
      const auto ow = _destination.w;
      const auto oh = _destination.h;
      const auto scale_factor = (1.0f - _scale) * .5f;
      const auto offset_x = ow * scale_factor;
      const auto offset_y = oh * scale_factor;
      const auto destination_x = _destination.x;
      const auto destination_y = _destination.y;
      _destination.x = destination_x + offset_x;
      _destination.y = destination_y + offset_y;
      _destination.w *= _scale;
      _destination.h *= _scale;
    }

    _redraw = false;
  }

  _spritesheet->draw(source.x, source.y, source.w, source.h, _destination.x, _destination.y, _destination.w, _destination.h, _angle, _alpha, _reflection);
}

void object::set_alpha(uint8_t alpha) noexcept {
  if (_alpha == alpha) [[likely]] return;

  const auto was_visible = _alpha != 0;
  const auto will_be_visible = alpha != 0;

  _alpha = alpha;

  if (was_visible != will_be_visible) {
    will_be_visible ? _body.enable() : _body.disable();
  }
}

void object::set_scale(float scale) noexcept {
  if (_scale == scale) [[likely]] return;

  _scale = scale;
  _dirty = true;
  _redraw = true;
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  if (_angle == angle) [[likely]] return;

  _angle = angle;
  _dirty = true;
  _redraw = true;
}

double object::angle() const noexcept {
  return _angle;
}

void object::set_reflection(enum reflection reflection) noexcept {
  _reflection = reflection;
}

reflection object::reflection() const noexcept {
  return _reflection;
}

bool object::visible() const noexcept {
  return _visible && !_action.empty() && _alpha != 0;
}

void object::set_visible(bool value) {
  if (value == _visible) [[likely]] return;

  _visible = value;
  value ? _body.enable() : _body.disable();
}

void object::set_action(std::optional<std::string_view> action) {
  if (!action || action->empty()) [[unlikely]] {
    _action.clear();
    _animation.reset();
    _body.disable();
    _dirty = true;
    _redraw = true;
    return;
  }

  if (_action == *action && _animation) [[likely]] return;

  const auto it = _animations.find(*action);
  if (it == _animations.end()) [[unlikely]] return;

  _action = it->first;
  _animation.emplace(it->second);
  _dirty = true;
  _redraw = true;

  if (auto& fx = _animation->_animation.effect; fx) [[likely]] {
    fx->play();
  }

  if (auto fn = _onbegin; fn) {
    fn(shared_from_this(), _action);
  }
}

std::string_view object::action() const noexcept {
  return _action;
}

void object::set_onbegin(sol::protected_function fn) {
  _onbegin = interop::wrap_fn<void(std::shared_ptr<object>, std::string_view)>(std::move(fn));
}

void object::set_onend(sol::protected_function fn) {
  _onend = interop::wrap_fn<void(std::shared_ptr<object>, std::string_view)>(std::move(fn));
}

void object::set_onmail(sol::protected_function fn) {
  _onmail = interop::wrap_fn<void(std::shared_ptr<object>, std::string_view)>(std::move(fn));
}

void object::set_ontouch(sol::protected_function fn) {
  _ontouch = interop::wrap_fn<void(std::shared_ptr<object>, float, float)>(std::move(fn));
}

void object::set_onhover(sol::protected_function fn) {
  _onhover = interop::wrap_fn<void(std::shared_ptr<object>)>(std::move(fn));
}

void object::set_onunhover(sol::protected_function fn) {
  _onunhover = interop::wrap_fn<void(std::shared_ptr<object>)>(std::move(fn));
}

void object::set_oncollision(std::string_view kind, sol::protected_function fn) {
  _collision_mapping.insert_or_assign(
    std::string{kind}, interop::wrap_fn<void(std::shared_ptr<object>, std::shared_ptr<object>)>(fn));
}

void object::on_email(std::string_view message) {
  if (auto fn = _onmail; fn) {
    fn(shared_from_this(), message);
  }
}

void object::on_touch(float x, float y) {
  if (auto fn = _ontouch; fn) {
    fn(shared_from_this(), x, y);
  }
}

void object::on_hover() {
  if (auto fn = _onhover; fn) {
    fn(shared_from_this());
  }
}

void object::on_unhover() {
  if (auto fn = _onunhover; fn) {
    fn(shared_from_this());
  }
}

kv& object::kv() noexcept {
  return _kv;
}

uint64_t object::id() const noexcept {
  return _id;
}

void object::suspend() {
  _body.disable();
}
