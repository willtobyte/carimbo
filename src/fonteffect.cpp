#include "fonteffect.hpp"

using namespace graphics;

void fadeineffect::set(const std::string& text, geometry::point position) {
  const auto length = text.size();
  if (length != _last_length) {
    _last_length = length;
    _text = text;
    _position = position;
    _fade_time = .0f;
    _animating = true;
    _alpha = 0;
  }

   _draw_calls = 0;
}

void fadeineffect::update(float delta) noexcept {
  if (_text.empty()) {
    return;
  }

  if (!_animating) {
    return;
  }

  _fade_time += delta;
  if (_fade_time >= _fade_duration) {
    _fade_time = _fade_duration;
    _animating = false;
  }

  const auto progress = _fade_time / _fade_duration;
  _alpha = static_cast<uint8_t>(progress * 255);
}

uint8_t fadeineffect::alpha() noexcept {
  ++_draw_calls;

  if (_draw_calls == _text.size()) {
    return _alpha;
  }

  return 255;
}
