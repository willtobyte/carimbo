#include "object.hpp"
#include "reflection.hpp"

using namespace framework;

object::object(const objectprops &props)
    : _props(props) {}

object::~object() {
  fmt::println("[object] destroyed {} {}", kind(), id());
}

uint64_t object::id() const {
  return _props.id;
}

std::string object::kind() const {
  return _props.kind;
}

std::string object::scope() const {
  return _props.scope;
}

objectprops &object::props() {
  return _props;
}

const objectprops &object::props() const {
  return _props;
}

geometry::point object::position() const {
  return _props.position;
}

float_t object::x() const {
  return _props.position.x();
}

void object::set_x(float_t x) {
  _props.position.set_x(x);
}

float_t object::y() const {
  return _props.position.y();
}

void object::set_y(float_t y) {
  _props.position.set_y(y);
}

void object::move(float_t x_velocity, float_t y_velocity) {
  UNUSED(x_velocity);
  UNUSED(y_velocity);
}

void object::set_velocity(const algebra::vector2d &velocity) {
  _props.velocity = velocity;
}

algebra::vector2d object::velocity() const {
  return _props.velocity;
}

void object::update(float_t delta) {
  if (_onupdate) {
    _onupdate(shared_from_this());
  }

  if (_props.action.empty()) {
    return;
  }

  const auto now = SDL_GetTicks();
  const auto it = _props.animations.find(_props.action);
  if (it == _props.animations.end()) {
    return;
  }

  auto& animation = it->second;
  auto& keyframes = animation.keyframes;
  if (_props.frame >= keyframes.size()) {
    return;
  }

  const auto& frame = keyframes[_props.frame];
  const bool expired = frame.duration > 0 && (now - _props.last_frame >= frame.duration);
  if (!expired) {
    _props.position.set(
      _props.position.x() + _props.velocity.x() * delta,
      _props.position.y() + _props.velocity.y() * delta
    );

    return;
  }

  _props.last_frame = now;
  ++_props.frame;

  if (_props.frame < keyframes.size()) {
    _props.position.set(
      _props.position.x() + _props.velocity.x() * delta,
      _props.position.y() + _props.velocity.y() * delta
    );

    return;
  }

  if (animation.oneshot) {
    const std::string finished = std::exchange(_props.action, "");

    if (_onanimationfinished) {
      _onanimationfinished(shared_from_this(), finished);
    }

    if (!animation.next) {
      return;
    }

    _props.action = *animation.next;
    _props.frame = 0;
    _props.last_frame = SDL_GetTicks();
  } else {
    _props.frame = 0;
  }

  _props.position.set(
    _props.position.x() + _props.velocity.x() * delta,
    _props.position.y() + _props.velocity.y() * delta
  );
}

void object::draw() const {
  if (_props.action.empty()) [[unlikely]] {
    return;
  }

  const auto &animation = _props.animations.at(_props.action).keyframes.at(_props.frame);
  const auto &source = animation.frame;
  const auto &offset = animation.offset;
#ifdef HITBOX
  const auto &hitbox = _props.animations.at(_props.action).hitbox;
#endif

  geometry::rectangle destination{_props.position + offset, source.size()};

  destination.scale(_props.scale);

#ifdef HITBOX
  const auto debug = hitbox
                         ? std::make_optional(geometry::rectangle{_props.position + hitbox->position(), hitbox->size() * _props.scale})
                         : std::nullopt;
#endif

  _props.spritesheet->draw(
      source,
      destination,
      _props.angle,
      _props.reflection,
      _props.alpha
#ifdef HITBOX
      ,
      debug
#endif
  );
}

void object::set_props(const objectprops &props) {
  _props = props;
}

void object::hide() {
  unset_action();
}

void object::set_placement(float_t x, float_t y) {
  _props.position.set(x, y);
}

geometry::point object::placement() const {
  return _props.position;
}

void object::set_alpha(uint8_t alpha) {
  _props.alpha = alpha;
}

uint8_t object::alpha() const {
  return _props.alpha;
}

void object::set_scale(float_t scale) {
  _props.scale = scale;
}

float_t object::scale() const {
  return _props.scale;
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
  _props.reflection = reflection;
}

graphics::reflection object::reflection() const {
  return _props.reflection;
}

void object::set_action(const std::string& action) {
  _props.action = action;
  _props.frame = 0;
  _props.last_frame = SDL_GetTicks();

  const auto &a = _props.animations.at(_props.action);
  if (const auto &e = a.effect; e) {
    e->play();
  }
}

void object::unset_action() {
  _props.action.clear();
  _props.frame = 0;
  _props.last_frame = SDL_GetTicks();
}

std::string object::action() const {
  return _props.action;
}

bool object::intersects(const std::shared_ptr<object> other) const {
  if (_props.action.empty() || other->_props.action.empty()) [[likely]] {
    return false;
  }

  const auto sit = _props.animations.find(_props.action);
  if (sit == _props.animations.end() || !sit->second.hitbox) [[likely]] {
    return false;
  }

  const auto oit = other->_props.animations.find(other->_props.action);
  if (oit == other->_props.animations.end() || !oit->second.hitbox) [[likely]] {
    return false;
  }

  return geometry::rectangle(
    position() + sit->second.hitbox->position() * _props.scale,
    sit->second.hitbox->size() * _props.scale
  )
  .intersects(
    geometry::rectangle(
      other->position() + oit->second.hitbox->position() * other->_props.scale,
      oit->second.hitbox->size() * other->_props.scale
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
  const auto it = _props.animations.find(_props.action);
  if (it == _props.animations.end() || !it->second.hitbox) {
    return;
  }

  const auto &animation = it->second;
  const auto hitbox = geometry::rectangle{_props.position + animation.hitbox->position() * _props.scale, animation.hitbox->size() * _props.scale};
  const bool inside = hitbox.contains(x, y);
  if (inside != _props.hover) {
    _props.hover = inside;
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
