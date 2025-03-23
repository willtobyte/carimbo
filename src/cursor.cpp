#include "cursor.hpp"

using namespace graphics;

cursor::cursor(const std::string &name, std::shared_ptr<framework::resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
  SDL_ShowCursor(false);

  const auto buffer = storage::io::read(fmt::format("cursors/{}.json", name));
  const auto j = nlohmann::json::parse(buffer);

  _point = j["point"].get<geometry::point>();
  _spritesheet = _resourcemanager->pixmappool()->get(j["spritesheet"].get<std::string>());
  _animations.reserve(j["animations"].size());

  _animations.reserve(j["animations"].size());

  for (const auto &item : j["animations"].items()) {
    const auto &key = item.key();
    const auto &a = item.value();
    const auto &f = a["frames"];
    std::vector<graphics::keyframe> keyframes(f.size());
    std::ranges::transform(f, keyframes.begin(), [](const auto &frame) {
      return graphics::keyframe{
          frame["rect"].template get<geometry::rect>(),
          frame.value("offset", geometry::point{}),
          frame["duration"].template get<uint64_t>(),
      };
    });

    const auto oneshot = a.value("oneshot", false);

    _animations.emplace(key, graphics::animation{oneshot, std::nullopt, keyframes});
  }
}

void cursor::on_mousemotion(const input::mousemotionevent &event) noexcept {
  _position = geometry::point{event.x, event.y};
}

void cursor::on_mousebuttondown(const input::mousebuttonevent &event) noexcept {
  // TODO FIX ME using enum input::mousebuttonevent::button;

  constexpr auto left = input::mousebuttonevent::button::left;
  constexpr auto right = input::mousebuttonevent::button::right;
  switch (event.button) {
  case left:
    _action = ACTION_LEFT;
    break;
  case right:
    _action = ACTION_RIGHT;
    break;
  default:
    return;
  }

  _frame = 0;
  _last_frame = SDL_GetTicks();
}

void cursor::on_mousebuttonup(const input::mousebuttonevent &event) noexcept {
  UNUSED(event);
}

void cursor::update(float_t) noexcept {
  const auto now = SDL_GetTicks();
  const auto &animation = _animations.at(_action);
  const auto &frame = animation.keyframes[_frame];

  if (frame.duration == 0 || now - _last_frame < frame.duration) {
    return;
  }

  _last_frame = now;

  if (animation.oneshot && (_frame + 1 >= animation.keyframes.size())) {
    _action = std::exchange(_queued_action, std::nullopt).value_or(ACTION_IDLE);
    _frame = 0;
    return;
  }

  _frame = (_frame + 1) % animation.keyframes.size();
}

void cursor::draw() const noexcept {
  const auto &animation = _animations.at(_action).keyframes[_frame];
  _spritesheet->draw(
      animation.frame,
      geometry::rect{_position - _point + animation.offset, animation.frame.size()},
      0,
      graphics::reflection::none,
      255
#ifdef HITBOX
      ,
      std::nullopt
#endif
  );
}

void cursor::handle(const std::string &message) noexcept {
  _queued_action = message;
}
