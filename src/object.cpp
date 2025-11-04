#include "object.hpp"

using namespace framework;

object::object() noexcept
  : _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(graphics::reflection::none),
    _body(b2_nullBodyId),
    _collision_shape(b2_nullShapeId) {
}

object::~object() noexcept {
  if (b2Body_IsValid(_body)) {
    b2DestroyBody(_body);
  }

  std::println("[object] destroyed {} {}", kind(), id());
}

std::string object::kind() const noexcept {
  return _kind;
}

std::string object::scope() const noexcept {
  return _scope;
}

geometry::point object::position() const noexcept {
  return _position;
}

float object::x() const noexcept {
  return _position.x();
}

void object::set_x(float x) noexcept {
  if (_position.x() == x) return;
  _position.set_x(x);
  sync_body();
}

float object::y() const noexcept {
  return _position.y();
}

void object::set_y(float y) noexcept {
  if (_position.y() == y) return;
  _position.set_y(y);
  sync_body();
}

void object::set_placement(float x, float y) noexcept {
  if (_position.x() == x && _position.y() == y) return;
  _position.set(x, y);
  sync_body();
}

geometry::point object::placement() const noexcept {
  return _position;
}

void object::apply_velocity(float vx, float vy) noexcept {
  if (!b2Body_IsValid(_body)) return;
  b2Body_SetLinearVelocity(_body, b2Vec2{vx, vy});
}

void object::update(float delta, uint64_t now) noexcept {
  const auto it = _animations.find(_action);
  if (it == _animations.end()) [[unlikely]] return;

  const auto& animation = it->second;
  const auto& keyframes = animation.keyframes;
  if (keyframes.empty() || _frame >= keyframes.size()) [[unlikely]] return;

  const auto& keyframe = keyframes[_frame];
  const auto expired = keyframe.duration > 0 && (now - _last_frame >= keyframe.duration);
  if (expired) {
    ++_frame;

    auto proceed = true;
    const auto done = _frame >= keyframes.size();
    if (done && animation.oneshot) {
      const auto ended = std::exchange(_action, std::string{});
      if (const auto& fn = _onend; fn) fn(shared_from_this(), ended);
      if (!animation.next) proceed = false;
      else _action = *animation.next;
    }

    if (proceed) {
      if (done) _frame = 0;
      _last_frame = now;
    }
  }

  if (!animation.bounds) {
    if (b2Body_IsValid(_body)) {
      b2DestroyBody(_body);
      _body = b2_nullBodyId;
    }

    return;
  }

  const auto pose = compute_pose();
  if (!pose) {
    if (b2Body_IsValid(_body)) {
      b2DestroyBody(_body);
      _body = b2_nullBodyId;
    }

    return;
  }

  if (!b2Body_IsValid(_body)) {
    if (auto world = _world.lock()) {
      auto def = b2DefaultBodyDef();
      def.type = b2_dynamicBody;
      def.gravityScale = 0.0f;
      def.fixedRotation = true;
      def.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(id()));
      def.position = b2Vec2{pose->px, pose->py};
      def.rotation = b2MakeRot(pose->radians);
      _body = b2CreateBody(*world, &def);

      auto sd = b2DefaultShapeDef();
      sd.isSensor = true;
      sd.enableSensorEvents = true;
      const auto box = b2MakeOffsetBox(pose->hx, pose->hy, b2Vec2{0.f, 0.f}, b2MakeRot(0));
      _collision_shape = b2CreatePolygonShape(_body, &sd, &box);

      _last_synced_scale = _scale;
      _last_synced_hx = pose->hx;
      _last_synced_hy = pose->hy;
    }
  } else {
    sync_body();
  }
}

void object::draw() const noexcept {
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

void object::set_alpha(uint8_t alpha) noexcept {
  if (_alpha == alpha) return;
  _alpha = alpha;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_scale(float scale) noexcept {
  if (_scale == scale) return;
  _scale = scale;
  sync_body();
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  if (_angle == angle) return;
  _angle = angle;
  sync_body();
}

double object::angle() const noexcept {
  if (b2Body_IsValid(_body)) {
    const auto rot = b2Body_GetRotation(_body);
    const auto radians = b2Rot_GetAngle(rot);
    return radians * (180.0 / std::numbers::pi_v<double>);
  }
  return _angle;
}

void object::set_reflection(graphics::reflection reflection) noexcept {
  if (_reflection == reflection) return;
  _reflection = reflection;
}

graphics::reflection object::reflection() const noexcept {
  return _reflection;
}

bool object::visible() const noexcept {
  return !_action.empty() || _alpha != 0 || !_visible;
}

void object::set_visible(bool value) noexcept {
  if (value == _visible) return;
  _visible = value;
}

void object::set_action(const std::optional<std::string>& action) noexcept {
  if (!action || action->empty()) {
    _action.clear();
    if (b2Body_IsValid(_body)) {
      b2DestroyBody(_body);
      _body = b2_nullBodyId;
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

std::string object::action() const noexcept {
  return _action;
}

void object::set_onbegin(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept {
  _onbegin = std::move(fn);
}

void object::set_onend(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept {
  _onend = std::move(fn);
}

void object::set_onmail(std::function<void(std::shared_ptr<object>, const std::string& )>&& fn) noexcept {
  _onmail = std::move(fn);
}

void object::set_ontouch(std::function<void(std::shared_ptr<object>, float, float)>&& fn) noexcept {
  _ontouch = std::move(fn);
}

void object::set_onhover(std::function<void(std::shared_ptr<object>)>&& fn) noexcept {
  _onhover = std::move(fn);
}

void object::set_onunhover(std::function<void(std::shared_ptr<object>)>&& fn) noexcept {
  _onunhover = std::move(fn);
}

void object::set_oncollision(const std::string& kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)>&& fn) noexcept {
  _collision_mapping.emplace(kind, std::move(fn));
}

void object::on_email(const std::string& message) noexcept {
  if (const auto& fn = _onmail; fn) {
    fn(shared_from_this(), message);
  }
}

void object::on_touch(float x, float y) noexcept {
  if (const auto& fn = _ontouch; fn) {
    fn(shared_from_this(), x, y);
  }
}

void object::on_hover() noexcept {
  if (const auto& fn = _onhover; fn) {
    fn(shared_from_this());
  }
}

void object::on_unhover() noexcept {
  if (const auto& fn = _onunhover; fn) {
    fn(shared_from_this());
  }
}

memory::kv& object::kv() noexcept {
  return _kv;
}

uint64_t object::id() const noexcept {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
}

std::optional<pose> object::compute_pose() const noexcept {
  const auto it = _animations.find(_action);
  if (it == _animations.end()) return std::nullopt;
  const auto& animation = it->second;
  if (!animation.bounds) return std::nullopt;
  const auto& r = animation.bounds->rectangle;
  const auto sw = r.width() * _scale;
  const auto sh = r.height() * _scale;
  const auto hx = 0.5f * sw;
  const auto hy = 0.5f * sh;
  const auto px = _position.x() + r.x() * _scale + hx;
  const auto py = _position.y() + r.y() * _scale + hy;
  const auto radians = static_cast<float>(_angle * (std::numbers::pi_v<float> / 180.0f));
  return pose{px, py, radians, hx, hy};
}

void object::sync_body() noexcept {
  if (!b2Body_IsValid(_body)) return;

  const auto pose = compute_pose();
  if (!pose) return;

  b2Body_SetTransform(_body, b2Vec2{pose->px, pose->py}, b2MakeRot(pose->radians));

  const auto epsilon = std::numeric_limits<float>::epsilon();
  const bool changed =
    (std::abs(_last_synced_scale - _scale) > epsilon) ||
    (std::abs(_last_synced_hx - pose->hx) > epsilon) ||
    (std::abs(_last_synced_hy - pose->hy) > epsilon);

  if (changed) {
    if (b2Shape_IsValid(_collision_shape)) {
      const auto box = b2MakeBox(pose->hx, pose->hy);
      b2Shape_SetPolygon(_collision_shape, &box);
    } else {
      auto sd = b2DefaultShapeDef();
      sd.isSensor = true;
      sd.enableSensorEvents = true;
      const auto box = b2MakeOffsetBox(pose->hx, pose->hy, b2Vec2{0.f, 0.f}, b2MakeRot(0));
      _collision_shape = b2CreatePolygonShape(_body, &sd, &box);
    }

    _last_synced_scale = _scale;
    _last_synced_hx = pose->hx;
    _last_synced_hy = pose->hy;
  }
}
