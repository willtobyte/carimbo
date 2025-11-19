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

object::object()
  : _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(graphics::reflection::none),
    _body(b2_nullBodyId),
    _collision_shape(b2_nullShapeId) {
  _collision_mapping.reserve(8);
}

object::~object() {
  if (b2Shape_IsValid(_collision_shape)) {
    b2DestroyShape(_collision_shape, false);
  }

  if (b2Body_IsValid(_body)) {
    b2DestroyBody(_body);
  }

  std::println("[object] destroyed {} {}", kind(), id());
}

std::string_view object::kind() const {
  return _kind;
}

std::string_view object::scope() const {
  return _scope;
}

geometry::point object::position() const {
  return _position;
}

float object::x() const {
  return _position.x();
}

void object::set_x(float x) {
  if (_position.x() == x) return;
  _position.set_x(x);
  _dirty = true;
}

float object::y() const {
  return _position.y();
}

void object::set_y(float y) {
  if (_position.y() == y) return;
  _position.set_y(y);
  _dirty = true;
}

void object::set_placement(float x, float y) {
  if (_position.x() == x && _position.y() == y) return;
  _position.set(x, y);
  _dirty = true;
}

geometry::point object::placement() const {
  return _position;
}

void object::update(float delta, uint64_t now) {
  if (!_current_animation) [[unlikely]] return;

  auto& animation = _current_animation->get();
  const auto& keyframes = animation.keyframes;
  if (!keyframes.empty() && _frame < keyframes.size()) {
    const auto& keyframe = keyframes[_frame];

    if (keyframe.duration > 0 && (now - _last_frame >= keyframe.duration)) {
      ++_frame;

      if (_frame >= keyframes.size()) {
        if (animation.oneshot) {
          const auto ended = std::exchange(_action, std::string{});
          if (const auto& fn = _onend; fn) fn(shared_from_this(), ended);
          if (!animation.next) return;
          set_action(*animation.next);
          return;
        }
        _frame = 0;
      }

      _last_frame = now;
    }
  }

  if (!animation.bounds || !_visible) [[unlikely]] {
    if (b2Body_IsValid(_body) && b2Body_IsEnabled(_body)) {
      b2Body_Disable(_body);
    }
    return;
  }

  if (b2Body_IsValid(_body) && !_dirty) {
    if (!b2Body_IsEnabled(_body)) b2Body_Enable(_body);
    return;
  }

  if (b2Body_IsValid(_body) || _dirty) {
    const auto& rectangle = animation.bounds->rectangle;
    const auto transform = physics::body_transform::compute(
      _position.x() + rectangle.x(), _position.y() + rectangle.y(),
      0, 0, rectangle.width(), rectangle.height(),
      _scale, _angle
    );

    const auto box = b2MakeBox(transform.hx(), transform.hy());
    const auto rotation = b2MakeRot(transform.radians());
    const auto position = b2Vec2{transform.px(), transform.py()};

    if (!b2Body_IsValid(_body)) {
      const auto world = _world.lock();

      if (b2Shape_IsValid(_collision_shape)) {
        std::println("[object] warning: orphaned shape for {} {}", kind(), id());
        b2DestroyShape(_collision_shape, false);
      }

      auto def = b2DefaultBodyDef();
      def.type = b2_kinematicBody;
      def.userData = physics::id_to_userdata(id());
      def.position = position;
      def.rotation = rotation;
      _body = b2CreateBody(*world, &def);

      auto sd = b2DefaultShapeDef();
      sd.isSensor = true;
      sd.enableSensorEvents = true;
      sd.filter.categoryBits = physics::collisioncategory::Player;
      sd.filter.maskBits = physics::collisioncategory::Player;
      _collision_shape = b2CreatePolygonShape(_body, &sd, &box);

      _dirty = false;
      return;
    }

    if (!b2Body_IsEnabled(_body)) b2Body_Enable(_body);

    b2Body_SetTransform(_body, position, rotation);

    if (b2Shape_IsValid(_collision_shape))
      b2Shape_SetPolygon(_collision_shape, &box);

    _dirty = false;
  }
}

void object::draw() const {
  if (!_visible || !_current_animation) [[unlikely]] return;

  const auto& keyframes = _current_animation->get().keyframes;
  if (keyframes.empty() || _frame >= keyframes.size()) [[unlikely]] {
    return;
  }

  const auto& keyframe = keyframes[_frame];
  const auto& source = keyframe.frame;
  const auto& offset = keyframe.offset;

  geometry::rectangle destination{_position + offset, source.size()};

  const auto ow = destination.width();
  const auto oh = destination.height();
  const auto sw = ow * _scale;
  const auto sh = oh * _scale;
  const auto dx = (ow - sw) * .5f;
  const auto dy = (oh - sh) * .5f;

  destination.set_position(destination.x() + dx, destination.y() + dy);
  destination.scale(_scale);

  _spritesheet->draw(
    source,
    destination,
    _angle,
    _alpha,
    _reflection
  );
}

void object::set_alpha(uint8_t alpha) {
  if (_alpha == alpha) return;
  _alpha = alpha;
}

uint8_t object::alpha() const {
  return _alpha;
}

void object::set_scale(float scale) {
  if (_scale == scale) return;
  _scale = scale;
  _dirty = true;
}

float object::scale() const {
  return _scale;
}

void object::set_angle(double angle) {
  if (_angle == angle) return;
  _angle = angle;
  _dirty = true;
}

double object::angle() const {
  return _angle;
}

void object::set_reflection(graphics::reflection reflection) {
  if (_reflection == reflection) return;
  _reflection = reflection;
}

graphics::reflection object::reflection() const {
  return _reflection;
}

bool object::visible() const {
  return !_action.empty() || _alpha != 0 || !_visible;
}

void object::set_visible(bool value) {
  if (value == _visible) return;
  _visible = value;

  if (b2Body_IsValid(_body)) {
    if (_visible) {
      b2Body_Enable(_body);
    } else {
      b2Body_Disable(_body);
    }
  }
}

void object::set_action(std::optional<std::string_view> action) {
  if (!action || action->empty()) {
    _action.clear();
    _current_animation = std::nullopt;

    if (b2Body_IsValid(_body) && b2Body_IsEnabled(_body)) {
      b2Body_Disable(_body);
    }

    return;
  }

  const auto it = _animations.find(*action);
  if (it == _animations.end()) [[unlikely]] {
    return;
  }

  _action = it->first;
  _current_animation = it->second;
  _frame = 0;
  _last_frame = SDL_GetTicks();

  _dirty = true;

  if (const auto& e = _current_animation->get().effect; e) {
    e->play();
  }

  if (const auto& fn = _onbegin; fn) {
    fn(shared_from_this(), _action);
  }
}

std::string_view object::action() const {
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
  using collision_fn = void(std::shared_ptr<object>, std::shared_ptr<object>);
  _collision_mapping.insert_or_assign(std::string{kind}, interop::wrap_fn<collision_fn>(fn));
}

void object::on_email(std::string_view message) {
  if (const auto& fn = _onmail; fn) {
    fn(shared_from_this(), message);
  }
}

void object::on_touch(float x, float y) {
  if (const auto& fn = _ontouch; fn) {
    fn(shared_from_this(), x, y);
  }
}

void object::on_hover() {
  if (const auto& fn = _onhover; fn) {
    fn(shared_from_this());
  }
}

void object::on_unhover() {
  if (const auto& fn = _onunhover; fn) {
    fn(shared_from_this());
  }
}

memory::kv& object::kv() noexcept {
  return _kv;
}

uint64_t object::id() const noexcept {
  return _id;
}

void object::suspend() {
  if (b2Body_IsValid(_body) && b2Body_IsEnabled(_body)) {
    b2Body_Disable(_body);
  }
}
