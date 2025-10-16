#include "object.hpp"

using namespace framework;

object::object() noexcept
  : _visible(true),
    _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(graphics::reflection::none) {
}

object::~object() noexcept {
  std::println("[object] gone {} {}", kind(), id());
}

uint64_t object::id() const noexcept {
  return _id;
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
  _dirty = true;
}

float object::y() const noexcept {
  return _position.y();
}

void object::set_y(float y) noexcept {
  if (_position.y() == y) return;
  _position.set_y(y);
  _dirty = true;
}

void object::set_velocity(const algebra::vector2d& velocity) noexcept {
  _velocity = velocity;
}

algebra::vector2d& object::velocity() noexcept {
  return _velocity;
}

void object::advance(const float delta) noexcept {
  if (_velocity.zero()) return;

  _position.set(
    _position.x() + _velocity.x() * delta,
    _position.y() + _velocity.y() * delta
  );
  _dirty = true;
}

void object::update(const float delta) noexcept {
  if (!_visible || _action.empty()) [[unlikely]] {
    return;
  }

  if (const auto& fn = _onupdate; fn) {
    fn(shared_from_this());
  }

  const auto it = _animations.find(_action);
  if (it == _animations.end()) [[unlikely]] {
    return;
  }

  const auto& animation  = it->second;
  const auto& keyframes  = animation.keyframes;
  if (keyframes.empty() || _frame >= keyframes.size()) [[unlikely]] {
    return;
  }

  _current_rectangle = animation.bounds ? animation.bounds->rectangle : geometry::rectangle{};

  const auto now = SDL_GetTicks();
  const auto& frame = keyframes[_frame];
  const bool expired = frame.duration > 0 && (now - _last_frame >= frame.duration);

  if (!expired) [[likely]] {
    advance(delta);
    return;
  }

  _last_frame = now;
  ++_frame;

  if (_frame < keyframes.size()) [[likely]] {
    advance(delta);
    return;
  }

  if (!animation.oneshot) {
    _frame = 0;
    advance(delta);
    return;
  }

  const auto ended = std::exchange(_action, "");
  if (const auto& fn = _onend; fn) {
    fn(shared_from_this(), ended);
  }

  if (!animation.next) return;

  _action = *animation.next;
  _frame = 0;
  _last_frame = SDL_GetTicks();

  advance(delta);
}

void object::draw() const noexcept {
  const auto it = _animations.find(_action);
  if (!_visible || it == _animations.end()) [[unlikely]] {
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

  #ifdef DEBUG
  std::optional<geometry::rectangle> debug = boundingbox();
  #endif

  _spritesheet->draw(
    source,
    destination,
    _angle,
    _alpha,
    _reflection
    #ifdef DEBUG
      , debug
    #endif
  );
}

void object::set_placement(float x, float y) noexcept {
  if (_position.x() == x && _position.y() == y) return;
  _position.set(x, y);
  _dirty = true;
}

geometry::point object::placement() const noexcept {
  return _position;
}

void object::set_alpha(uint8_t alpha) noexcept {
  if (_alpha == alpha) return;
  _alpha = alpha;
  _dirty = true;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_scale(float scale) noexcept {
  if (_scale == scale) return;
  _scale = scale;
  _dirty = true;
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  if (_angle == angle) return;
  _angle = angle;
  _dirty = true;
}

double object::angle() const noexcept {
  return _angle;
}

void object::set_reflection(graphics::reflection reflection) noexcept {
  if (_reflection == reflection) return;
  _reflection = reflection;
  _dirty = true;
}

graphics::reflection object::reflection() const noexcept {
  return _reflection;
}

void object::set_visible(bool value) noexcept {
  if (_visible == value) return;
  _visible = value;
  _dirty = true;
}

bool object::visible() const noexcept {
  return _visible;
}

void object::set_action(const std::optional<std::string>& action) noexcept {
  if (!action.has_value()) {
    unset_action();
    return;
  }

  if (_action == *action) {
    return;
  }

  _action = *action;
  _frame = 0;
  _last_frame = SDL_GetTicks();
  _dirty = true;
  _visible = true;

  const auto& animation = _animations.at(_action);
  if (const auto& e = animation.effect; e) {
    e->play();
  }

  if (const auto& fn = _onbegin; fn) {
    fn(shared_from_this(), _action);
  }
}

void object::unset_action() noexcept {
  _action.clear();
  _frame = 0;
  _last_frame = SDL_GetTicks();
  _visible = false;
  _dirty = true;
}

std::string object::action() const noexcept {
  return _action;
}

geometry::rectangle object::boundingbox() const noexcept {
  return {
    _position + _current_rectangle.position() * _scale,
    _current_rectangle.size() * _scale
  };
}

void object::set_onupdate(std::function<void(std::shared_ptr<object>)>&& fn) noexcept {
  _onupdate = std::move(fn);
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
  _collisionmapping.emplace(kind, std::move(fn));
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

bool object::dirty() noexcept{
  return std::exchange(_dirty, false);
}

memory::kv& object::kv() noexcept {
  return _kv;
}
