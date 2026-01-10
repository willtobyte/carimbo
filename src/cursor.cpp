#include "cursor.hpp"

#include "flip.hpp"
#include "geometry.hpp"
#include "io.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

cursor::cursor(std::string_view name, std::shared_ptr<renderer> renderer) {
  SDL_HideCursor();

  auto json = unmarshal::parse(io::read(std::format("cursors/{}.json", name)));

  _point = json["point"].get<vec2>();

  _spritesheet = std::make_shared<pixmap>(std::move(renderer), std::format("blobs/overlay/{}.png", name));

  if (auto animations = json["animations"]) {
    animations.foreach([this](std::string_view key, unmarshal::json node) {
      auto [it, inserted] = _animations.try_emplace(key);
      if (!inserted) {
        return;
      }

      const auto oneshot = node["oneshot"].get(false);

      boost::container::small_vector<keyframe, 16> frames;
      const auto fnode = node["frames"];
      frames.reserve(fnode.size());
      fnode.foreach([&frames](unmarshal::json node) {
        frames.emplace_back(std::move(node));
      });

      it->second = animation{oneshot, std::nullopt, nullptr, std::move(frames)};
    });
  }

  if (const auto it = _animations.find(ACTION_DEFAULT); it != _animations.end()) {
    _current_animation = &it->second;
  }
}

cursor::~cursor() {
  SDL_ShowCursor();
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
