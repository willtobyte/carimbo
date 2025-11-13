#include "cursor.hpp"

#include "eventreceiver.hpp"
#include "io.hpp"
#include "pixmap.hpp"
#include "pixmappool.hpp"
#include "point.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "resourcemanager.hpp"

using namespace graphics;

using namespace input::event;

cursor::cursor(std::string_view name, std::shared_ptr<framework::resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
  SDL_HideCursor();

  const auto filename = std::format("cursors/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  _point = j["point"].template get<geometry::point>();
  _spritesheet = _resourcemanager->pixmappool()->get(std::format("blobs/overlay/{}.png", name));
  _animations.reserve(j["animations"].size());

  for (const auto& item : j["animations"].items()) {
    const auto& key = item.key();
    const auto& a = item.value();
    const auto& f = a["frames"];
    std::vector<graphics::keyframe> keyframes(f.size());
    std::ranges::transform(f, keyframes.begin(), [](const auto& frame) {
      return graphics::keyframe{
          frame["rectangle"].template get<geometry::rectangle>(),
          frame["offset"].template get<geometry::point>(),
          frame["duration"].template get<uint64_t>(),
      };
    });

    const auto oneshot = a.value("oneshot", false);

    _animations.emplace(key, graphics::animation{oneshot, keyframes});
  }

  if (const auto it = _animations.find(ACTION_DEFAULT); it != _animations.end()) {
    _current_animation = it->second;
  }
}

void cursor::on_mouse_release(const mouse::button& event) {
  constexpr auto left = mouse::button::which::left;
  constexpr auto middle = mouse::button::which::middle;
  constexpr auto right = mouse::button::which::right;

  switch (event.button) {
  case left:
    _action = ACTION_LEFT;
    break;
  case middle:
    break;
  case right:
    _action = ACTION_RIGHT;
    break;
  }

  if (const auto it = _animations.find(_action); it != _animations.end()) {
    _current_animation = it->second;
  }

  _frame = 0;
  _last_frame = SDL_GetTicks();
}

void cursor::on_mouse_motion(const mouse::motion& event) {
  _position = geometry::point(event.x, event.y);
}

void cursor::update(float delta) {
  if (!_current_animation) [[unlikely]] return;

  const auto now = SDL_GetTicks();
  auto& animation = _current_animation->get();
  const auto& keyframes = animation.keyframes;

  if (_frame >= keyframes.size()) [[unlikely]] return;

  const auto& frame = keyframes[_frame];

  if (frame.duration == 0 || now - _last_frame < frame.duration) {
    return;
  }

  _last_frame = now;

  if (animation.oneshot && (_frame + 1 >= keyframes.size())) {
    _action = std::exchange(_queued_action, std::nullopt).value_or(ACTION_DEFAULT);
    if (const auto it = _animations.find(_action); it != _animations.end()) {
      _current_animation = it->second;
    }
    _frame = 0;
    return;
  }

  _frame = (_frame + 1) % keyframes.size();
}

void cursor::draw() const {
  if (!_current_animation) [[unlikely]] return;

  const auto& keyframes = _current_animation->get().keyframes;
  if (_frame >= keyframes.size()) [[unlikely]] return;

  const auto& keyframe = keyframes[_frame];

  _spritesheet->draw(
      keyframe.frame,
      geometry::rectangle(
        _position - _point + keyframe.offset,
        keyframe.frame.size()
      ),
      0,
      255,
      reflection::none
  );
}

void cursor::handle(const std::string_view message) {
  _queued_action = message;
}
