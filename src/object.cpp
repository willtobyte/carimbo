#include "object.hpp"

#include "constant.hpp"
#include "kv.hpp"
#include "physics.hpp"
#include "reflection.hpp"
#include "world.hpp"

object::controller::controller(animation& animation) : _animation(animation), _last_tick(0) {
  refresh();
}

void object::controller::refresh() {
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
  refresh();
  return true;
}

void object::controller::reset() {
  _frame = 0;
  _last_tick = 0;
  refresh();
}

bool object::controller::finished() const noexcept {
  return _frame >= _animation.keyframes.size();
}

bool object::controller::valid() const noexcept {
  return _has_keyframe;
}

const quad& object::controller::bounds() const noexcept {
  return _bounds;
}

bool object::body::sync_state::changed(const vec2& pos, const quad& b, float s, double a) const noexcept {
  return !valid || position != pos || bounds != b || scale != s || angle != a;
}

object::body::body(std::weak_ptr<world> world) : _world(std::move(world)) {}

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

bool object::body::valid() const noexcept {
  return b2Body_IsValid(_id);
}

void object::body::toggle(bool enable) {
  if (!b2Body_IsValid(_id) || _enabled == enable) [[unlikely]] return;

  enable ? b2Body_Enable(_id) : b2Body_Disable(_id);
  _enabled = enable;
}

void object::body::create(const quad& bounds, const vec2& position, float scale, double angle, uint64_t id) {
  auto world = _world.lock();
  if (!world) [[unlikely]] return;

  const auto sw = bounds.w * scale;
  const auto sh = bounds.h * scale;
  const auto center_x = bounds.w * 0.5f;
  const auto center_y = bounds.h * 0.5f;
  const auto px = position.x + bounds.x + center_x;
  const auto py = position.y + bounds.y + center_y;
  const auto radians = static_cast<float>(angle) * DEGREES_TO_RADIANS;

  auto def = b2DefaultBodyDef();
  def.type = b2_kinematicBody;
  def.userData = id_to_userdata(id);
  def.position = b2Vec2{px, py};
  def.rotation = b2MakeRot(radians);
  _id = b2CreateBody(*world, &def);

  auto sd = b2DefaultShapeDef();
  sd.isSensor = true;
  sd.enableSensorEvents = true;
  sd.filter.categoryBits = collisioncategory::player;
  sd.filter.maskBits = collisioncategory::player;

  const auto box = b2MakeBox(sw * 0.5f, sh * 0.5f);
  _shape = b2CreatePolygonShape(_id, &sd, &box);
  _enabled = true;
}

void object::body::transform(const quad& bounds, const vec2& position, float scale, double angle) {
  const auto sw = bounds.w * scale;
  const auto sh = bounds.h * scale;
  const auto center_x = bounds.w * 0.5f;
  const auto center_y = bounds.h * 0.5f;
  const auto px = position.x + bounds.x + center_x;
  const auto py = position.y + bounds.y + center_y;
  const auto radians = static_cast<float>(angle) * DEGREES_TO_RADIANS;

  toggle(true);
  b2Body_SetTransform(_id, b2Vec2{px, py}, b2MakeRot(radians));

  if (b2Shape_IsValid(_shape)) [[likely]] {
    const auto box = b2MakeBox(sw * 0.5f, sh * 0.5f);
    b2Shape_SetPolygon(_shape, &box);
  }
}

void object::body::sync(const quad& bounds, const vec2& position, float scale, double angle, uint64_t id) {
  if (!_last_sync.changed(position, bounds, scale, angle)) [[likely]] return;

  if (!valid()) [[unlikely]] {
    create(bounds, position, scale, angle, id);
  } else {
    transform(bounds, position, scale, angle);
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

void object::set_position(float x, float y) noexcept {
  if (_position.x == x && _position.y == y) [[unlikely]] return;

  _position.x = x;
  _position.y = y;
  mark_dirty();
}

void object::set_x(float x) noexcept {
  set_position(x, _position.y);
}

void object::set_y(float y) noexcept {
  set_position(_position.x, y);
}

void object::set_placement(float x, float y) noexcept {
  set_position(x, y);
}

void object::mark_dirty() noexcept {
  _dirty = true;
  _redraw = true;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_scale(float scale) noexcept {
  if (_scale == scale) [[likely]] return;

  _scale = scale;
  mark_dirty();
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  if (_angle == angle) [[likely]] return;

  _angle = angle;
  mark_dirty();
}

double object::angle() const noexcept {
  return _angle;
}

enum reflection object::reflection() const noexcept {
  return _reflection;
}

void object::set_reflection(enum reflection reflection) noexcept {
  _reflection = reflection;
}

void object::set_alpha(uint8_t alpha) noexcept {
  if (_alpha == alpha) [[likely]] return;

  const auto was_visible = _alpha != 0;
  const auto will_be_visible = alpha != 0;

  _alpha = alpha;

  if (was_visible != will_be_visible) {
    _body.toggle(will_be_visible);
  }
}

bool object::visible() const noexcept {
  return _visible && !_action.empty() && _alpha != 0;
}

void object::set_visible(bool value) {
  if (value == _visible) [[likely]] return;

  _visible = value;
  _body.toggle(value);
}

std::string_view object::action() const noexcept {
  return _action;
}

kv& object::kv() noexcept {
  return _kv;
}

uint64_t object::id() const noexcept {
  return _id;
}

void object::update(float delta, uint64_t now) {
  if (!_controller) [[unlikely]] return;

  const auto changed = _controller->tick(now);

  if (!changed) [[likely]] {
    if (_controller->_has_bounds && _visible) [[likely]] {
      if (_body.valid() && !_dirty) [[likely]] {
        _body.toggle(true);
        return;
      }

      _body.sync(_controller->bounds(), _position, _scale, _angle, _id);
      _dirty = false;
      return;
    }

    _body.toggle(false);
    return;
  }

  _redraw = true;

  if (!_controller->finished()) [[likely]] {
    if (_controller->_has_bounds && _visible) [[likely]] {
      _body.sync(_controller->bounds(), _position, _scale, _angle, _id);
      _dirty = false;
      return;
    }

    _body.toggle(false);
    return;
  }

  // Animation finished
  if (!_controller->_animation.oneshot) [[unlikely]] {
    _controller->reset();
    return;
  }

  if (auto fn = _onend; fn) {
    fn(shared_from_this(), _action);
  }

  if (_controller->_animation.next) [[likely]] {
    set_action(*_controller->_animation.next);
  }
}

void object::draw() const {
  if (!_visible || !_controller || !_controller->valid()) [[unlikely]] return;

  const auto& source = _controller->_source;
  const auto& offset = _controller->_offset;

  if (_redraw) [[unlikely]] {
    _destination = quad{_position + offset, {source.w, source.h}};

    if (_scale != 1.0f) [[unlikely]] {
      const auto scale_factor = (1.0f - _scale) * .5f;
      const auto offset_x = _destination.w * scale_factor;
      const auto offset_y = _destination.h * scale_factor;
      _destination.x += offset_x;
      _destination.y += offset_y;
      _destination.w *= _scale;
      _destination.h *= _scale;
    }

    _redraw = false;
  }

  _spritesheet->draw(source.x, source.y, source.w, source.h,
                     _destination.x, _destination.y, _destination.w, _destination.h,
                     _angle, _alpha, _reflection);
}

void object::set_action(std::optional<std::string_view> action) {
  if (!action || action->empty()) [[unlikely]] {
    _action.clear();
    _controller.reset();
    _body.toggle(false);
    mark_dirty();
    return;
  }

  if (_action == *action && _controller) [[likely]] return;

  const auto it = _animations.find(*action);
  if (it == _animations.end()) [[unlikely]] return;

  _action = it->first;
  _controller.emplace(it->second);
  mark_dirty();

  if (auto& fx = _controller->_animation.effect; fx) [[likely]] {
    fx->play();
  }

  if (auto fn = _onbegin; fn) {
    fn(shared_from_this(), _action);
  }
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

void object::suspend() {
  _body.toggle(false);
}
