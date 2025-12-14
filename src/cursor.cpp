#include "cursor.hpp"

#include "io.hpp"
#include "flip.hpp"
#include "resourcemanager.hpp"
#include "geometry.hpp"

cursor::cursor(std::string_view name, std::shared_ptr<resourcemanager> resourcemanager)
    : _resourcemanager(std::move(resourcemanager)) {
  SDL_HideCursor();

  const auto filename = std::format("cursors/{}.json", name);
  auto json = unmarshal::parse(io::read(filename)); auto& document = *json;

  unmarshal::value p;
  document["point"].get(p);
  from_json(p, _point);

  _spritesheet = _resourcemanager->pixmappool()->get(std::format("blobs/overlay/{}.png", name));

  for (auto field : document["animations"].get_object()) {
    const auto key = unmarshal::key(field);
    auto aobject = unmarshal::get<unmarshal::object>(field.value());

    const auto oneshot = unmarshal::value_or(aobject, "oneshot", false);

    auto keyframes = boost::container::small_vector<keyframe, 16>{};
    if (auto frames = unmarshal::find_array(aobject, "frames")) {
      for (auto element : *frames) {
        auto k = keyframe{};
        k.duration = unmarshal::get<uint64_t>(element, "duration");
        k.offset = unmarshal::make<vec2>(element["offset"]);
        k.frame = unmarshal::make<quad>(element["quad"]);
        keyframes.emplace_back(std::move(k));
      }
    }

    _animations.emplace(key, animation{oneshot, std::nullopt, nullptr, keyframes});
  }

  if (const auto it = _animations.find(ACTION_DEFAULT); it != _animations.end()) {
    _current_animation = &it->second;
  }
}

void cursor::on_mouse_release(const event::mouse::button& event) {
  constexpr auto left = event::mouse::button::which::left;
  constexpr auto middle = event::mouse::button::which::middle;
  constexpr auto right = event::mouse::button::which::right;

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
    _current_animation = &it->second;
  }

  _frame = 0;
  _last_frame = SDL_GetTicks();
}

void cursor::on_mouse_motion(const event::mouse::motion& event) {
  _position = vec2(event.x, event.y);
}

void cursor::update(float delta) {
  if (!_current_animation) [[unlikely]] return;

  const auto now = SDL_GetTicks();
  auto& animation = *_current_animation;
  const auto& keyframes = animation.keyframes;

  if (_frame >= keyframes.size()) [[unlikely]] return;

  const auto& frame = keyframes[_frame];

  if (frame.duration == 0 || now - _last_frame < frame.duration) {
    return;
  }

  _last_frame = now;

  if (animation.oneshot && (_frame + 1 >= keyframes.size())) {
    _action = _queued_action.empty() ? ACTION_DEFAULT : _queued_action;
    _queued_action.clear();
    if (const auto it = _animations.find(_action); it != _animations.end()) {
      _current_animation = &it->second;
    }
    _frame = 0;
    return;
  }

  _frame = (_frame + 1) % keyframes.size();
}

void cursor::draw() const {
  if (!_current_animation) [[unlikely]] return;

  const auto& keyframes = _current_animation->keyframes;
  if (_frame >= keyframes.size()) [[unlikely]] return;

  const auto& keyframe = keyframes[_frame];

  const auto position = _position - _point + keyframe.offset;

  _spritesheet->draw(
      keyframe.frame.x, keyframe.frame.y,
      keyframe.frame.w, keyframe.frame.h,
      position.x, position.y,
      keyframe.frame.w, keyframe.frame.h,
      0,
      255,
      flip::none
  );
}

void cursor::handle(std::string_view message) {
  _queued_action = message;
}
