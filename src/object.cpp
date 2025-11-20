#include "object.hpp"

#include "kv.hpp"
#include "physics.hpp"
#include "pixmap.hpp"
#include "point.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "soundfx.hpp"
#include "world.hpp"

using namespace framework;

object::controller::controller(animation& a, uint64_t now) : _animation(a), _last_tick(now) {
  frooze();
}

void object::controller::frooze() {
  const auto& keyframes = _animation.keyframes;

  if (_frame >= keyframes.size()) [[unlikely]] {
    _keyframe = nullptr;
    _source = nullptr;
    _offset = nullptr;
    _bounds = nullptr;
    return;
  }

  _keyframe = &keyframes[_frame];
  _source = &_keyframe->frame;
  _offset = &_keyframe->offset;
  _bounds = _animation.bounds ? &(*_animation.bounds) : nullptr;
}

bool object::controller::tick(uint64_t now) {
  if (!_keyframe || _keyframe->duration == 0) [[unlikely]] return false;
  if (now - _last_tick < _keyframe->duration) [[likely]] return false;

  ++_frame;
  _last_tick = now;
  frooze();
  return true;
}

void object::controller::reset(uint64_t now) {
  _frame = 0;
  _last_tick = now;
  frooze();
}

bool object::controller::finished() const {
  return _frame >= _animation.keyframes.size();
}

bool object::controller::valid() const {
  return _keyframe != nullptr;
}

const geometry::rectangle& object::controller::bounds() const {
  return _bounds->rectangle;
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

bool object::body::missing() const {
  return !b2Body_IsValid(_id);
}

bool object::body::valid() const {
  return b2Body_IsValid(_id);
}

void object::body::sync(const geometry::rectangle& bounds, const geometry::point& position,
                        float scale, double angle, uint64_t id, std::weak_ptr<world> world) {
  const auto transform = physics::body_transform::compute(
    position.x() + bounds.x(), position.y() + bounds.y(),
    0, 0, bounds.width(), bounds.height(), scale, angle
  );

  const auto box = b2MakeBox(transform.hx(), transform.hy());
  const auto rotation = b2MakeRot(transform.radians());
  const auto b2_position = b2Vec2{transform.px(), transform.py()};

  if (missing()) [[unlikely]] {
    auto w = world.lock();
    if (!w) [[unlikely]] return;

    auto def = b2DefaultBodyDef();
    def.type = b2_kinematicBody;
    def.userData = physics::id_to_userdata(id);
    def.position = b2_position;
    def.rotation = rotation;
    _id = b2CreateBody(*w, &def);

    auto sd = b2DefaultShapeDef();
    sd.isSensor = true;
    sd.enableSensorEvents = true;
    sd.filter.categoryBits = physics::collisioncategory::Player;
    sd.filter.maskBits = physics::collisioncategory::Player;
    _shape = b2CreatePolygonShape(_id, &sd, &box);
    _enabled = true;
    return;
  }

  enable();
  b2Body_SetTransform(_id, b2_position, rotation);
  if (b2Shape_IsValid(_shape)) [[likely]] {
    b2Shape_SetPolygon(_shape, &box);
  }
}

object::object()
  : _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(graphics::reflection::none) {
  _collision_mapping.reserve(8);
}

object::~object() {
  std::println("[object] destroyed {} {}", kind(), id());
}

std::string_view object::kind() const { return _kind; }
std::string_view object::scope() const { return _scope; }
geometry::point object::position() const { return _position; }
float object::x() const { return _position.x(); }
float object::y() const { return _position.y(); }
geometry::point object::placement() const { return _position; }

void object::set_x(float x) {
  if (_position.x() == x) [[unlikely]] return;

  _position.set_x(x);
  _dirty = true;
}

void object::set_y(float y) {
  if (_position.y() == y) [[unlikely]] return;

  _position.set_y(y);
  _dirty = true;
}

void object::set_placement(float x, float y) {
  if (_position.x() == x && _position.y() == y) [[unlikely]] return;

  _position.set(x, y);
  _dirty = true;
}

void object::update(float delta, uint64_t now) {
  if (!_animation) [[unlikely]] return;

  if (!_animation->tick(now) || !_animation->finished()) [[likely]] {
    if (!_animation->_bounds || !_visible) [[unlikely]] {
      _body.disable();
      return;
    }

    if (_body.valid() && !_dirty) [[likely]] {
      _body.enable();
      return;
    }

    _body.sync(_animation->bounds(), _position, _scale, _angle, _id, _world);
    _dirty = false;
    return;
  }

  if (!_animation->_animation.oneshot) [[unlikely]] {
    _animation->reset(now);
    return;
  }

  const auto ended = std::exchange(_action, std::string{});
  if (auto fn = _onend; fn) fn(shared_from_this(), ended);
  if (_animation->_animation.next) [[likely]] set_action(*_animation->_animation.next);
}

void object::draw() const {
  if (!_visible || !_animation || !_animation->valid()) [[unlikely]] return;

  const auto& source = *_animation->_source;
  const auto& offset = *_animation->_offset;

  geometry::rectangle destination{_position + offset, source.size()};

  if (_scale != 1.0f) [[unlikely]] {
    const auto [ow, oh] = std::pair{destination.width(), destination.height()};
    const auto [sw, sh] = std::pair{ow * _scale, oh * _scale};
    destination.set_position(destination.x() + (ow - sw) * .5f, destination.y() + (oh - sh) * .5f);
    destination.scale(_scale);
  }

  _spritesheet->draw(source, destination, _angle, _alpha, _reflection);
}

void object::set_alpha(uint8_t alpha) { _alpha = alpha; }
uint8_t object::alpha() const { return _alpha; }

void object::set_scale(float scale) {
  if (_scale == scale) [[likely]] return;

  _scale = scale;
  _dirty = true;
}

float object::scale() const { return _scale; }

void object::set_angle(double angle) {
  if (_angle == angle) [[likely]] return;

  _angle = angle;
  _dirty = true;
}

double object::angle() const { return _angle; }

void object::set_reflection(graphics::reflection reflection) {
  _reflection = reflection;
}

graphics::reflection object::reflection() const { return _reflection; }

bool object::visible() const { return _visible && !_action.empty() && _alpha != 0; }

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
    return;
  }

  auto it = _animations.find(*action);
  if (it == _animations.end()) [[unlikely]] return;

  const auto now = SDL_GetTicks();
  _action = it->first;
  _animation.emplace(it->second, now);
  _dirty = true;

  if (auto& effect = _animation->_animation.effect) [[likely]] effect->play();
  if (auto fn = _onbegin; fn) fn(shared_from_this(), _action);
}

std::string_view object::action() const { return _action; }

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
    std::string{kind},
    interop::wrap_fn<void(std::shared_ptr<object>, std::shared_ptr<object>)>(fn)
  );
}

void object::on_email(std::string_view message) {
  if (auto fn = _onmail; fn) fn(shared_from_this(), message);
}

void object::on_touch(float x, float y) {
  if (auto fn = _ontouch; fn) fn(shared_from_this(), x, y);
}

void object::on_hover() {
  if (auto fn = _onhover; fn) fn(shared_from_this());
}

void object::on_unhover() {
  if (auto fn = _onunhover; fn) fn(shared_from_this());
}

memory::kv& object::kv() noexcept { return _kv; }
uint64_t object::id() const noexcept { return _id; }

void object::suspend() {
  _body.disable();
}
