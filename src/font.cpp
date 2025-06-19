#include "font.hpp"

using namespace graphics;

void fadeineffect::set(const std::string& text, geometry::point position) {
  const auto length = text.size();
  if (length != _last_length) {
    _last_length = length;
    _text = text;
    _position = position;
    _fade_time = 0.0f;
    _animating = true;
    _alpha = 0;
  }

   _draw_calls = 0;
}

void fadeineffect::update(float_t delta) noexcept {
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

font::font(
  const std::string& glyphs,
  const glyphmap& map,
  std::shared_ptr<pixmap> pixmap,
  int16_t spacing,
  int16_t leading,
  float_t scale
)
  : _glyphs(glyphs),
    _map(map),
    _pixmap(pixmap),
    _spacing(spacing),
    _leading(leading),
    _scale(scale)
{}

void font::draw(const std::string& text, const geometry::point& position, const std::weak_ptr<fonteffect>& effect) const noexcept {
  if (text.empty()) {
    return;
  }

  if (const auto e = effect.lock()) {
    e->set(text, position);
  }

  geometry::point cursor = position;

  const auto height = _map.begin()->second.height() * _scale;

  for (const auto ch : text) {
    if (ch == '\n') {
      cursor = geometry::point(position.x(), cursor.y() + height + _leading);
      continue;
    }

    const auto it = _map.find(static_cast<uint8_t>(ch));
    if (it == _map.end()) {
      continue;
    }

    const auto& glyph = it->second;
    const auto size = glyph.size();

    // TODO
    // float_t scale = .0f;
    // if (auto* e = _effect.get()) {
    //   scale = e->scale();
    // }

    double_t angle = .0L;
    if (const auto e = effect.lock()) {
      angle = e->angle();
    }

    reflection reflection = reflection::none;
    if (const auto e = effect.lock()) {
      reflection = e->reflection();
    }

    uint8_t alpha = 255;
    if (const auto e = effect.lock()) {
      alpha = e->alpha();
    }

    _pixmap->draw(
      glyph,
      {cursor, size * _scale},
      angle,
      reflection,
      alpha
    );

    cursor += std::make_pair('x', size.width() + _spacing);
  }
}

std::string font::glyphs() const {
  return _glyphs;
}
