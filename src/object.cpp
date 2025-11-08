#include "object.hpp"
#include "physics.hpp"

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

std::string object::kind() const {
  return _kind;
}

std::string object::scope() const {
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
  _need_update_physics = true;
}

float object::y() const {
  return _position.y();
}

void object::set_y(float y) {
  if (_position.y() == y) return;
  _position.set_y(y);
  _need_update_physics = true;
}

void object::set_placement(float x, float y) {
  if (_position.x() == x && _position.y() == y) return;
  _position.set(x, y);
  _need_update_physics = true;
}

geometry::point object::placement() const {
  return _position;
}

void object::update(float delta, uint64_t now) {
  const auto it = _animations.find(_action);
  if (it == _animations.end()) [[unlikely]] return;

  const auto& animation = it->second;
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
          _action = *animation.next;
        }
        _frame = 0;
      }

      _last_frame = now;
    }
  }

  if (!animation.bounds || !_visible) [[unlikely]] {
    if (b2Body_IsValid(_body) && b2Body_IsEnabled(_body)) b2Body_Disable(_body);
    return;
  }

  if (b2Body_IsValid(_body) && !_need_update_physics) {
    if (!b2Body_IsEnabled(_body)) b2Body_Enable(_body);
    return;
  }

  const auto& rectangle = animation.bounds->rectangle;
  const auto transform = physics::body_transform::compute(
    _position.x() + rectangle.x(), _position.y() + rectangle.y(),
    0, 0, rectangle.width(), rectangle.height(),
    _scale, _angle
  );

  if (!b2Body_IsValid(_body)) {
    const auto world = _world.lock();

    if (b2Shape_IsValid(_collision_shape)) {
      std::println("[object] warning: orphaned shape for {} {}", kind(), id());
      b2DestroyShape(_collision_shape, false);
      _collision_shape = b2_nullShapeId;
    }

    auto def = b2DefaultBodyDef();
    def.type = b2_kinematicBody;
    def.userData = physics::id_to_userdata(id());
    def.position = b2Vec2{transform.px, transform.py};
    def.rotation = b2MakeRot(transform.radians);
    _body = b2CreateBody(*world, &def);

    auto sd = b2DefaultShapeDef();
    sd.isSensor = true;
    sd.enableSensorEvents = true;
    sd.filter.categoryBits = physics::collisioncategory::Player; // TODO change it
    sd.filter.maskBits = physics::collisioncategory::Player; // TODO change it
    const auto box = b2MakeBox(transform.hx, transform.hy);
    _collision_shape = b2CreatePolygonShape(_body, &sd, &box);
    _last_synced_transform = transform;
    return;
  }

  if (!b2Body_IsEnabled(_body)) b2Body_Enable(_body);

  _need_update_physics = false;

  b2Body_SetTransform(_body, b2Vec2{transform.px, transform.py}, b2MakeRot(transform.radians));

  if (!_last_synced_transform || transform.shape_differs(*_last_synced_transform)) {
    if (b2Shape_IsValid(_collision_shape)) {
      const auto box = b2MakeBox(transform.hx, transform.hy);
      b2Shape_SetPolygon(_collision_shape, &box);
    }
  }

  _last_synced_transform = transform;
}

void object::draw() const {
  if (!_visible) [[unlikely]] return;

  const auto it = _animations.find(_action);
  if (it == _animations.end()) [[unlikely]] {
    return;
  }

  const auto& keyframes = it->second.keyframes;
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
  _need_update_physics = true;
}

float object::scale() const {
  return _scale;
}

void object::set_angle(double angle) {
  if (_angle == angle) return;
  _angle = angle;
  _need_update_physics = true;
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

void object::set_action(const std::optional<std::string>& action) {
  if (!action || action->empty()) {
    _action.clear();
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
  _frame = 0;
  _last_frame = SDL_GetTicks();

  if (const auto& e = it->second.effect; e) {
    e->play();
  }

  if (const auto& fn = _onbegin; fn) {
    fn(shared_from_this(), _action);
  }
}

std::string object::action() const {
  return _action;
}

void object::set_onbegin(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) {
  _onbegin = std::move(fn);
}

void object::set_onend(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) {
  _onend = std::move(fn);
}

void object::set_onmail(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) {
  _onmail = std::move(fn);
}

void object::set_ontouch(std::function<void(std::shared_ptr<object>, float, float)>&& fn) {
  _ontouch = std::move(fn);
}

void object::set_onhover(std::function<void(std::shared_ptr<object>)>&& fn) {
  _onhover = std::move(fn);
}

void object::set_onunhover(std::function<void(std::shared_ptr<object>)>&& fn) {
  _onunhover = std::move(fn);
}

void object::set_oncollision(const std::string& kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>&& fn) {
  _collision_mapping.emplace(kind, std::move(fn));
}

void object::on_email(const std::string& message) {
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

memory::kv& object::kv() {
  return _kv;
}

uint64_t object::id() const {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
}

void object::suspend() {
  if (b2Body_IsValid(_body) && b2Body_IsEnabled(_body)) {
    b2Body_Disable(_body);
  }
}
