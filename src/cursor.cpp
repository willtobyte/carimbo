#include "cursor.hpp"

using namespace graphics;

cursor::cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager)
    : _x(0), _y(0), _resourcemanager(resourcemanager) {
  // SDL_ShowCursor(false);
  //  SDL_SetRelativeMouseMode(true);

  _action = "idle";

  const auto buffer = storage::io::read(fmt::format("cursors/{}.json", name));
  const auto j = nlohmann::json::parse(buffer);

  _size = j["size"].get<geometry::size>();
  _spritesheet = _resourcemanager->pixmappool()->get(j["spritesheet"].get_ref<const std::string &>());
  _animations.reserve(j["animations"].size());
  for (const auto &[key, anim] : j["animations"].items()) {
    std::vector<graphics::keyframe> keyframes;
    keyframes.reserve(anim["frames"].size());
    for (const auto &frame : anim["frames"]) {
      keyframes.emplace_back(
          graphics::keyframe{
              frame["rect"].get<geometry::rect>(),
              frame.value("offset", geometry::point{}),
              frame["duration"].get<uint64_t>(),
              frame.value("singleshoot", bool{false})
          }
      );
    }
    _animations.emplace(key, graphics::animation{std::nullopt, std::move(keyframes)});
  }
}

void cursor::on_mousemotion(const input::mousemotionevent &event) noexcept {
  const auto [x, y] = event;
  _x = x;
  _y = y;
}

void cursor::on_mousebuttondown(const input::mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void cursor::on_mousebuttonup(const input::mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void cursor::update(float_t delta) noexcept {
  UNUSED(delta);

  const auto now = SDL_GetTicks();
  const auto &animation = _animations.at(_action);
  const auto &frame = animation.keyframes.at(_frame);

  if (frame.duration == 0 || now - _last_frame < frame.duration) {
    return;
  }

  _last_frame = now;
  _frame = (_frame + 1) % animation.keyframes.size();
}

void cursor::draw() const noexcept {
  const auto &animation = _animations.at(_action).keyframes.at(_frame);
  const auto &source = animation.frame;
  const auto &offset = animation.offset;

  geometry::rect destination{geometry::point{_x, _y} + offset, source.size()};

  _spritesheet->draw(
      source,
      destination,
      0,
      graphics::reflection::none,
      255
#ifdef HITBOX
      ,
      std::nullopt
#endif
  );

  /*
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

  */
}
