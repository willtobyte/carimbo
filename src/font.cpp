#include "font.hpp"

using namespace graphics;

void fadeineffect::set(const std::string &text, geometry::point position) {
  _text = text;
  _position = position;
}

void fadeineffect::update(float_t delta) {
  const auto new_last = _text.back();
  if (new_last != _last_char) {
    _last_char = new_last;
    _fade_time = 0.0f;
    _animating = true;
  }

  _alpha = 255;
  if (_animating && _index + 1 == _text.size()) {
    float_t progress = _fade_time / _fade_duration;
    _alpha = progress < 1.0f ? static_cast<uint8_t>(progress * 255) : 255;
  }
}

uint8_t fadeineffect::alpha() {
  return _alpha;
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

  _effect->set(text, position);

  geometry::point cursor = position;

  const auto height = _glyphs.begin()->second.size().height() * _scale;

  for (size_t index = 0; index < text.size(); ++index) {
    auto c = text[index];
    if (c == '\n') {
      cursor = geometry::point(position.x(), cursor.y() + height + _leading);
      continue;
    }

    const auto& glyph = _glyphs.at(static_cast<uint8_t>(c));

    auto size = glyph.size();

    float_t scale = 1.f;
    if (auto* e = _effect.get())
        alpha = e->scale();

    reflection reflection = reflection::none;
    if (auto* e = _effect.get())
        reflection = e->reflection();

    uint8_t alpha = 255;
    if (auto* e = _effect.get())
        alpha = e->alpha();

    _pixmap->draw(
      glyph,
      {cursor, size * _scale},
      scale,
      reflection,
      alpha
    );

    cursor += std::make_pair('x', size.width() + _spacing);
  }
}

// void font::draw(const std::string &text, const geometry::point &position) const {
//   geometry::point cursor = position;
//   const auto height = _glyphs.begin()->second.size().height() * _scale;

//   for (const char character : text) {
//     switch (character) {
//       case '\n':
//         cursor = geometry::point(position.x(), cursor.y() + height + _leading);
//         break;

//       default: {
//         const auto &glyph = _glyphs.at(static_cast<uint8_t>(character));
//         auto size = glyph.size();
//         _pixmap->draw(glyph, {cursor, size * _scale});
//         cursor += std::make_pair('x', size.width() + _spacing);
//         break;
//       }
//     }
//   }
// }
