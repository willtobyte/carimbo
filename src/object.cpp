#include "object.hpp"
#include "reflection.hpp"

using namespace framework;

object::object()
  : _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(.0),
    _hover(false)
{}

object::~object() {
  fmt::println("[object] destroyed {} {}", kind(), id());
}

uint64_t object::id() const {
  return _id;
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

float_t object::x() const {
  return _position.x();
}

void object::set_x(float_t x) {
  _position.set_x(x);
}

float_t object::y() const {
  return _position.y();
}

void object::set_y(float_t y) {
  _position.set_y(y);
}

void object::move(float_t x_velocity, float_t y_velocity) {
  UNUSED(x_velocity);
  UNUSED(y_velocity);
}

void object::set_velocity(const algebra::vector2d &velocity) {
  _velocity = velocity;
}

algebra::vector2d object::velocity() const {
  return _velocity;
}

void object::update(float_t delta) {
  if (_onupdate) {
    _onupdate(shared_from_this());
  }

  if (_action.empty()) {
    return;
  }

  const auto now = SDL_GetTicks();
  const auto it = _animations.find(_action);
  if (it == _animations.end()) {
    return;
  }

  const auto &animation = it->second;
  const auto &keyframes = animation.keyframes;
  if (_frame >= keyframes.size()) {
    return;
  }

  const auto &frame = keyframes[_frame];
  const bool expired = frame.duration > 0 && (now - _last_frame >= frame.duration);
  if (!expired) {
    _position.set(
      _position.x() + _velocity.x() * delta,
      _position.y() + _velocity.y() * delta
    );

    return;
  }

  _last_frame = now;
  ++_frame;

  if (_frame < keyframes.size()) {
    _position.set(
      _position.x() + _velocity.x() * delta,
      _position.y() + _velocity.y() * delta
    );

    return;
  }

  if (animation.oneshot) {
    const std::string finished = std::exchange(_action, "");

    if (_onanimationfinished) {
      _onanimationfinished(shared_from_this(), finished);
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

  _position.set(
    _position.x() + _velocity.x() * delta,
    _position.y() + _velocity.y() * delta
  );
}

void object::draw() const {
  if (_action.empty()) [[unlikely]] {
    return;
  }

  const auto &animation = _animations.at(_action).keyframes.at(_frame);
  const auto &source = animation.frame;
  const auto &offset = animation.offset;
#ifdef HITBOX
  const auto &hitbox = _animations.at(_action).hitbox;
#endif

  geometry::rectangle destination{_position + offset, source.size()};

  destination.scale(_scale);

#ifdef HITBOX
  const auto debug = hitbox
                         ? std::make_optional(geometry::rectangle{_position + hitbox->position(), hitbox->size() * _scale})
                         : std::nullopt;
#endif

  _spritesheet->draw(
      source,
      destination,
      _angle,
      _reflection,
      _alpha
#ifdef HITBOX
      ,
      debug
#endif
  );
}

void object::hide() {
  unset_action();
}

void object::set_placement(float_t x, float_t y) {
  _position.set(x, y);
}

geometry::point object::placement() const {
  return _position;
}

void object::set_alpha(uint8_t alpha) {
  _alpha = alpha;
}

uint8_t object::alpha() const {
  return _alpha;
}

void object::set_scale(float_t scale) {
  _scale = scale;
}

float_t object::scale() const {
  return _scale;
}

void object::set_onupdate(std::function<void(std::shared_ptr<object>)> fn) {
  _onupdate = std::move(fn);
}

void object::set_onanimationfinished(std::function<void(std::shared_ptr<object>, const std::string &)> fn) {
  _onanimationfinished = std::move(fn);
}

void object::set_onmail(std::function<void(std::shared_ptr<object>, const std::string &)> fn) {
  _onmail = std::move(fn);
}

void object::set_ontouch(std::function<void(std::shared_ptr<object>, float_t, float_t)> fn) {
  _ontouch = std::move(fn);
}

void object::set_onhover(std::function<void(std::shared_ptr<object>)> fn) {
  _onhover = std::move(fn);
}

void object::set_onunhover(std::function<void(std::shared_ptr<object>)> fn) {
  _onunhover = std::move(fn);
}

void object::set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)> fn) {
  _collisionmapping.emplace(kind, std::move(fn));
}

void object::set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)> fn) {
  _tickinmapping.emplace(n, std::move(fn));
}

void object::set_reflection(graphics::reflection reflection) {
  _reflection = reflection;
}

graphics::reflection object::reflection() const {
  return _reflection;
}

void object::set_action(const std::string& action) {
  _action = action;
  _frame = 0;
  _last_frame = SDL_GetTicks();

  const auto &a = _animations.at(_action);
  if (const auto &e = a.effect; e) {
    e->play();
  }
}

void object::unset_action() {
  _action.clear();
  _frame = 0;
  _last_frame = SDL_GetTicks();
}

std::string object::action() const {
  return _action;
}

bool object::intersects(const std::shared_ptr<object> other) const {
  if (_action.empty() || other->_action.empty()) [[likely]] {
    return false;
  }

  const auto sit = _animations.find(_action);
  if (sit == _animations.end() || !sit->second.hitbox) [[likely]] {
    return false;
  }

  const auto oit = other->_animations.find(other->_action);
  if (oit == other->_animations.end() || !oit->second.hitbox) [[likely]] {
    return false;
  }

  return geometry::rectangle(
    position() + sit->second.hitbox->position() * _scale,
    sit->second.hitbox->size() * _scale
  )
  .intersects(
    geometry::rectangle(
      other->position() + oit->second.hitbox->position() * other->_scale,
      oit->second.hitbox->size() * other->_scale
    )
  );
}

void object::on_email(const std::string &message) {
  if (const auto fn = _onmail; fn) {
    fn(shared_from_this(), message);
  }
}

void object::on_touch(float_t x, float_t y) {
  if (const auto fn = _ontouch; fn) {
    fn(shared_from_this(), x, y);
  }
}

void object::on_motion(float_t x, float_t y) {
  const auto it = _animations.find(_action);
  if (it == _animations.end() || !it->second.hitbox) {
    return;
  }

  const auto &animation = it->second;
  const auto hitbox = geometry::rectangle{_position + animation.hitbox->position() * _scale, animation.hitbox->size() * _scale};
  const bool inside = hitbox.contains(x, y);
  if (inside != _hover) {
    _hover = inside;
    inside ? on_hover() : on_unhover();
  }
}

void object::on_hover() {
  if (const auto fn = _onhover; fn) {
    fn(shared_from_this());
  }
}

void object::on_unhover() {
  if (const auto fn = _onunhover; fn) {
    fn(shared_from_this());
  }
}

memory::kv &object::kv() {
  return _kv;
}
