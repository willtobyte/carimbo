#include "object.hpp"
#include <SDL3/SDL_timer.h>

using namespace framework;

object::object(const objectprops &props) noexcept
    : _props(props) {}

object::~object() noexcept {
  fmt::println("[object] destroyed {} {}", kind(), id());
}

uint64_t object::id() const noexcept {
  return _props.id;
}

std::string object::kind() const noexcept {
  return _props.kind;
}

objectprops &object::props() noexcept {
  return _props;
}

const objectprops &object::props() const noexcept {
  return _props;
}

geometry::point object::position() const noexcept {
  return _props.position;
}

int32_t object::x() const noexcept {
  return _props.position.x();
}

int32_t object::y() const noexcept {
  return _props.position.y();
}

void object::move(float_t x_velocity, float_t y_velocity) noexcept {
  UNUSED(x_velocity);
  UNUSED(y_velocity);
}

void object::set_velocity(const algebra::vector2d &velocity) noexcept {
  _props.velocity = velocity;
}

algebra::vector2d object::velocity() const noexcept {
  return _props.velocity;
}

void object::update(float_t delta) noexcept {
  if (const auto fn = _onupdate; fn) {
    fn(shared_from_this());
  }

  if (_props.action.empty()) {
    _props.position.set(
        _props.position.x() + _props.velocity.x() * delta,
        _props.position.y() + _props.velocity.y() * delta
    );

    return;
  }

  const auto now = SDL_GetTicks();
  const auto &animation = _props.animations.at(_props.action);
  const auto &frame = animation.keyframes[_props.frame];

  if (frame.duration > 0 && now - _props.last_frame < frame.duration) {
    _props.position.set(
        _props.position.x() + _props.velocity.x() * delta,
        _props.position.y() + _props.velocity.y() * delta
    );

    return;
  }

  _props.last_frame = now;
  if (++_props.frame >= animation.keyframes.size()) {
    if (animation.oneshot) {
      auto finished = std::exchange(_props.action, "");
      if (const auto fn = _onanimationfinished; fn) {
        fn(shared_from_this(), finished);
      }
      if (animation.next.has_value()) {
        _props.action = animation.next.value();
        _props.frame = 0;
        _props.last_frame = 0;
      } else {
        return;
      }
    } else {
      _props.frame = 0;
    }
  }

  _props.position.set(
      _props.position.x() + _props.velocity.x() * delta,
      _props.position.y() + _props.velocity.y() * delta
  );
}

void object::draw() const noexcept {
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

void object::set_props(const objectprops &props) noexcept {
  _props = props;
}

void object::hide() noexcept {
  unset_action();
}

void object::set_placement(int32_t x, int32_t y) noexcept {
  _props.position.set(x, y);
}

geometry::point object::get_placement() const noexcept {
  return _props.position;
}

void object::set_onupdate(std::function<void(std::shared_ptr<object>)> fn) noexcept {
  _onupdate = std::move(fn);
}

void object::set_onanimationfinished(std::function<void(std::shared_ptr<object>, const std::string &)> fn) noexcept {
  _onanimationfinished = std::move(fn);
}

void object::set_onmail(std::function<void(std::shared_ptr<object>, const std::string &)> fn) noexcept {
  _onmail = std::move(fn);
}

void object::set_ontouch(std::function<void()> fn) noexcept {
  _ontouch = std::move(fn);
}

void object::set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<object>, std::shared_ptr<object>)> fn) noexcept {
  _collisionmapping.emplace(kind, std::move(fn));
}

void object::set_onnthtick(uint64_t n, std::function<void(std::shared_ptr<object>)> fn) noexcept {
  _tickinmapping.emplace(n, std::move(fn));
}

void object::set_reflection(graphics::reflection reflection) noexcept {
  _props.reflection = reflection;
}

void object::set_action(const std::string &action) noexcept {
  if (_props.action != action) {
    _props.action = action;
    _props.frame = 0;
    _props.last_frame = SDL_GetTicks();
  }
}

void object::unset_action() noexcept {
  _props.action.clear();
  _props.frame = 0;
  _props.last_frame = SDL_GetTicks();
}

std::string object::action() const noexcept {
  return _props.action;
}

bool object::intersects(const std::shared_ptr<object> other) const noexcept {
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

void object::on_touch() noexcept {
  if (const auto fn = _ontouch; fn) {
    fn();
  }
}

memory::kv &object::kv() noexcept {
  return _kv;
}
