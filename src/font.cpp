#include "font.hpp"

using namespace graphics;

void fadeineffect::set(const std::string &text, geometry::point position) {
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

void fadeineffect::update(float_t delta) {
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

uint8_t fadeineffect::alpha() {
  ++_draw_calls;

  if (_draw_calls == _text.size()) {
    return _alpha;
  }

  return 255;
}

font::font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale)
  : _glyphs(glyphs),
    _pixmap(pixmap),
    _spacing(spacing),
    _leading(leading),
    _scale(scale)
{}

void font::set_effect(fonteffect::type type) {
  switch (type) {
    case fonteffect::type::fadein:
      _effect = std::make_unique<fadeineffect>();
      break;
    default:
      break;
  }
}

void font::update(float_t delta) {
  if (_effect) {
    _effect->update(delta);
  }
}

void font::draw(const std::string& text, const geometry::point& position) const {
  if (text.empty()) {
    return;
  }

  if (auto* e = _effect.get()) {
    e->set(text, position);
  }

  geometry::point cursor = position;

  const auto height = _glyphs.begin()->second.size().height() * _scale;

  for (auto index = 0u; index < text.size(); ++index) {
    const auto ch = text[index];
    if (ch == '\n') {
      cursor = geometry::point(position.x(), cursor.y() + height + _leading);
      continue;
    }

    const auto& glyph = _glyphs.at(static_cast<uint8_t>(ch));

    auto size = glyph.size();

    // TODO
    // float_t scale = .0f;
    // if (auto* e = _effect.get()) {
    //   scale = e->scale();
    // }

    double_t angle = .0L;
    if (auto* e = _effect.get()) {
      angle = e->angle();
    }

    reflection reflection = reflection::none;
    if (auto* e = _effect.get()) {
      reflection = e->reflection();
    }

    uint8_t alpha = 255;
    if (auto* e = _effect.get()) {
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
