#include "object.hpp"

using namespace framework;

object::object() noexcept
  : _frame(0),
    _last_frame(SDL_GetTicks()),
    _angle(.0),
    _alpha(255),
    _scale(1.0f),
    _reflection(graphics::reflection::none) {
}

object::~object() noexcept {
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
  _needs_recalc = true;
}

float object::y() const noexcept {
  return _position.y();
}

void object::set_y(float y) noexcept {
  if (_position.y() == y) return;
  _position.set_y(y);
  _needs_recalc = true;
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

  if (!animation.bounds) [[unlikely]] {
    _shape = std::nullopt;
    return;
  }

  if (!_needs_recalc) [[likely]] {
    return;
  }

  const auto& rectangle = animation.bounds->rectangle;
  geometry::rectangle destination{_position + rectangle.position(), rectangle.size()};

  const auto ow = destination.width();
  const auto oh = destination.height();
  const auto sw = ow * _scale;
  const auto sh = oh * _scale;
  const auto dx = (ow - sw) * .5f;
  const auto dy = (oh - sh) * .5f;

  destination.set_position(destination.x() + dx, destination.y() + dy);
  destination.scale(_scale);

  const auto cx = destination.x() + sw * .5f;
  const auto cy = destination.y() + sh * .5f;

  const auto hx = sw * .5f;
  const auto hy = sh * .5f;

  const float r = _angle * (std::numbers::pi_v<float> / 180.0f);
  const float c = std::cos(r);
  const float s = std::sin(r);

  const auto ex = std::fma(std::abs(c), hx, std::abs(s) * hy);
  const auto ey = std::fma(std::abs(s), hx, std::abs(c) * hy);

  const auto minx = cx - ex;
  const auto maxx = cx + ex;
  const auto miny = cy - ey;
  const auto maxy = cy + ey;

  _shape = geometry::rectangle{minx, miny, maxx - minx, maxy - miny};
  _dirty = _shape != _previous_shape;
  _previous_shape = _shape;
  _needs_recalc = false;
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

void object::set_placement(float x, float y) noexcept {
  if (_position.x() == x && _position.y() == y) return;
  _position.set(x, y);
  _needs_recalc = true;
}

geometry::point object::placement() const noexcept {
  return _position;
}

void object::set_alpha(uint8_t alpha) noexcept {
  if (_alpha == alpha) return;
  _alpha = alpha;
  _needs_recalc = true;
}

uint8_t object::alpha() const noexcept {
  return _alpha;
}

void object::set_scale(float scale) noexcept {
  if (_scale == scale) return;
  _scale = scale;
  _needs_recalc = true;
}

float object::scale() const noexcept {
  return _scale;
}

void object::set_angle(double angle) noexcept {
  if (_angle == angle) return;
  _angle = angle;
  _needs_recalc = true;
}

double object::angle() const noexcept {
  return _angle;
}

void object::set_reflection(graphics::reflection reflection) noexcept {
  if (_reflection == reflection) return;
  _reflection = reflection;
  _needs_recalc = true;
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
  _needs_recalc = true;
}

void object::set_action(const std::optional<std::string>& action) noexcept {
  if (!action || action->empty()) {
    _action.clear();
    _needs_recalc = true;
    return;
  }

  const auto it = _animations.find(*action);
  if (it == _animations.end()) [[unlikely]] {
    return;
  }

  _action = it->first;
  _frame = 0;
  _last_frame = SDL_GetTicks();
  _needs_recalc = true;

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

bool object::dirty() noexcept{
  return std::exchange(_dirty, false);
}

std::optional<geometry::rectangle> object::shape() const {
  return _shape;
}

memory::kv& object::kv() noexcept {
  return _kv;
}

uint64_t object::id() const noexcept {
  return static_cast<uint64_t>(reinterpret_cast<uintptr_t>(this));
}
