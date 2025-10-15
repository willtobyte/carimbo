#include "object.hpp"

using namespace framework;

object::object() noexcept
  : _visible(true),
    _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(.0),
    _reflection(graphics::reflection::none),
    _hover(false) {
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
  _position.set_x(x);
}

float object::y() const noexcept {
  return _position.y();
}

void object::set_y(float y) noexcept {
  _position.set_y(y);
}

void object::set_velocity(const algebra::vector2d& velocity) noexcept {
  _velocity = velocity;
}

algebra::vector2d& object::velocity() noexcept {
  return _velocity;
}

void object::move(const float delta) noexcept {
  _position.set(
    _position.x() + _velocity.x() * delta,
    _position.y() + _velocity.y() * delta
  );
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
  if (_frame >= keyframes.size()) [[unlikely]] {
    return;
  }

  const auto now = SDL_GetTicks();
  const auto& frame = keyframes[_frame];
  const bool expired = frame.duration > 0 && (now - _last_frame >= frame.duration);

  if (!expired) [[likely]] {
    move(delta);
    return;
  }

  _last_frame = now;
  ++_frame;

  if (_frame < keyframes.size()) [[likely]] {
    move(delta);
    return;
  }

  if (animation.oneshot) {
    const auto action = std::exchange(_action, "");

    if (const auto& fn = _onend; fn) {
      fn(shared_from_this(), action);
    }

    if (!animation.next) {
      return;
    }

    _action = *animation.next;
    _frame = 0;
    _last_frame = SDL_GetTicks();
  } else {
    _frame = 0;
  }

  move(delta);
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
  _position.set(x, y);
}

geometry::point object::placement() const noexcept {
  return _position;
}

void object::set_alpha(uint8_t alpha) noexcept {
  _alpha = alpha;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_scale(float scale) noexcept {
  _scale = scale;
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  _angle = angle;
}

double object::angle() const noexcept {
  return _angle;
}

void object::set_reflection(graphics::reflection reflection) noexcept {
  _reflection = reflection;
}

graphics::reflection object::reflection() const noexcept {
  return _reflection;
}

void object::set_visible(bool value) noexcept {
  _visible = value;
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
}

std::string object::action() const noexcept {
  return _action;
}

std::optional<geometry::rectangle> object::boundingbox() const noexcept {
  std::optional<geometry::rectangle> result;

  const auto previous = _boundingbox;
    defer({
      const bool changed =
        (previous.has_value() != result.has_value()) ||
        (previous && result && (*previous != *result));

      _dirty = changed;
      _boundingbox = result;
    });

  if (!_visible || _action.empty()) [[unlikely]] {
    return std::nullopt;
  }

  const auto it = _animations.find(_action);
  if (it == _animations.end()) [[unlikely]] {
    return std::nullopt;
  }

  const auto& animation = it->second;
  if (!animation.bounds) [[unlikely]] {
    return std::nullopt;
  }

  const auto& bounds = *animation.bounds;
  const auto& rect = bounds.rectangle;
  result = geometry::rectangle{
    _position + rect.position() * _scale,
    rect.size() * _scale
  };

  return result;
}

void object::set_onupdate(std::function<void(std::shared_ptr<object>)>&& fn) {
  _onupdate = std::move(fn);
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
  _collisionmapping.emplace(kind, std::move(fn));
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
  if (_hover) {
    return;
  }

  _hover = true;

  if (const auto& fn = _onhover; fn) {
    fn(shared_from_this());
  }
}

void object::on_unhover() {
  if (!_hover) {
    return;
  }

  _hover = false;

  if (const auto& fn = _onunhover; fn) {
    fn(shared_from_this());
  }
}

bool object::dirty() noexcept {
  return std::exchange(_dirty, false);
}

memory::kv& object::kv() {
  return _kv;
}
