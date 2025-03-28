#include "entity.hpp"

using namespace framework;

entity::entity(const entityprops &props) noexcept
    : _props(props) {}

entity::~entity() noexcept {
  fmt::println("[entity] destroyed {} {}", kind(), id());
}

std::shared_ptr<entity> entity::create(const entityprops &props) {
  return std::make_shared<entity>(props);
}

uint64_t entity::id() const noexcept {
  return _props.id;
}

void entity::set_id(uint64_t id) noexcept {
  _props.id = id;
}

std::string entity::kind() const noexcept {
  return _props.kind;
}

entityprops &entity::props() noexcept {
  return _props;
}

const entityprops &entity::props() const noexcept {
  return _props;
}

geometry::point entity::position() const noexcept {
  return _props.position;
}

int32_t entity::x() const noexcept {
  return _props.position.x();
}

int32_t entity::y() const noexcept {
  return _props.position.y();
}

void entity::move(float_t x_velocity, float_t y_velocity) noexcept {
  UNUSED(x_velocity);
  UNUSED(y_velocity);
}

void entity::set_velocity(const algebra::vector2d &velocity) noexcept {
  _props.velocity = velocity;
}

algebra::vector2d entity::velocity() const noexcept {
  return _props.velocity;
}

void entity::update(float_t delta) noexcept {
  if (const auto fn = _onupdate; fn) {
    fn(shared_from_this());
  }

  if (_props.action.empty() || !_props.visible) {
    _props.position.set(
        static_cast<int32_t>(_props.position.x() + _props.velocity.x() * delta),
        static_cast<int32_t>(_props.position.y() + _props.velocity.y() * delta)
    );

    return;
  }

  const auto now = SDL_GetTicks();
  const auto &animation = _props.animations.at(_props.action);
  const auto &frame = animation.keyframes[_props.frame];

  if (frame.duration > 0 && now - _props.last_frame < frame.duration) {
    _props.position.set(
        static_cast<int32_t>(_props.position.x() + _props.velocity.x() * delta),
        static_cast<int32_t>(_props.position.y() + _props.velocity.y() * delta)
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

      return;
    }

    _props.frame = 0;
  }

  _props.position.set(
      static_cast<int32_t>(_props.position.x() + _props.velocity.x() * delta),
      static_cast<int32_t>(_props.position.y() + _props.velocity.y() * delta)
  );
}

void entity::draw() const noexcept {
  if (_props.action.empty() || !_props.visible) {
    return;
  }

  const auto &animation = _props.animations.at(_props.action).keyframes.at(_props.frame);
  const auto &source = animation.frame;
  const auto &offset = animation.offset;
#ifdef HITBOX
  const auto &hitbox = _props.animations.at(_props.action).hitbox;
#endif

  geometry::rect destination{_props.position + offset, source.size()};

  destination.scale(_props.scale);

#ifdef HITBOX
  const auto debug = hitbox
                         ? std::make_optional(geometry::rect{_props.position + hitbox->position(), hitbox->size() * _props.scale})
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

void entity::set_props(const entityprops &props) noexcept {
  _props = props;
}

void entity::set_placement(int32_t x, int32_t y) noexcept {
  _props.position.set(x, y);
}

geometry::point entity::get_placement() const noexcept {
  return _props.position;
}

void entity::set_onupdate(std::function<void(std::shared_ptr<entity>)> fn) noexcept {
  _onupdate = std::move(fn);
}

void entity::set_onanimationfinished(std::function<void(std::shared_ptr<entity>, const std::string &)> fn) noexcept {
  _onanimationfinished = std::move(fn);
}

void entity::set_onmail(std::function<void(std::shared_ptr<entity>, const std::string &)> fn) noexcept {
  _onmail = std::move(fn);
}

void entity::set_ontouch(std::function<void()> fn) noexcept {
  _ontouch = std::move(fn);
}

void entity::set_oncollision(const std::string &kind, std::function<void(std::shared_ptr<entity>, std::shared_ptr<entity>)> fn) noexcept {
  _collisionmapping.emplace(kind, std::move(fn));
}

void entity::set_onntick(uint64_t n, std::function<void(std::shared_ptr<entity>)> fn) noexcept {
  _tickinmapping.emplace(n, std::move(fn));
}

void entity::set_reflection(graphics::reflection reflection) noexcept {
  _props.reflection = reflection;
}

void entity::set_action(const std::string &action) noexcept {
  if (_props.action != action) {
    _props.action.assign(std::move(action));
    _props.frame = 0;
    _props.last_frame = SDL_GetTicks();
  }
}

std::string entity::get_action() const noexcept {
  return _props.action;
}

void entity::unset_action() noexcept {
  _props.action.clear();
  _props.frame = 0;
  _props.last_frame = SDL_GetTicks();
}

std::string entity::action() const noexcept {
  return _props.action;
}

bool entity::visible() const noexcept {
  return _props.visible;
}

bool entity::intersects(std::shared_ptr<entity> other) const noexcept {
  if (_props.action.empty() || other->_props.action.empty()) [[unlikely]] {
    return false;
  }

  const auto sit = _props.animations.find(_props.action);
  if (sit == _props.animations.end()) [[unlikely]] {
    return false;
  }

  const auto oit = other->_props.animations.find(other->_props.action);
  if (oit == other->_props.animations.end()) [[unlikely]] {
    return false;
  }

  const auto &hitbox = sit->second.hitbox;
  const auto &other_hitbox = oit->second.hitbox;
  if (!hitbox || !other_hitbox) [[likely]] {
    return false;
  }

  return geometry::rect(position() + hitbox->position() * _props.scale, hitbox->size() * _props.scale)
      .intersects(
          {other->position() + other_hitbox->position() * other->_props.scale,
           other_hitbox->size() * other->_props.scale
          }
      );
}

void entity::on_email(const std::string &message) {
  if (const auto fn = _onmail; fn) {
    fn(shared_from_this(), message);
  }
}

void entity::on_touch() noexcept {
  if (const auto fn = _ontouch; fn) {
    fn();
  }
}

memory::kv &entity::kv() noexcept {
  return _kv;
}
