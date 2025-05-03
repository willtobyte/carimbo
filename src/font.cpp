#include "font.hpp"

using namespace graphics;

font::font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale)
  : _glyphs(glyphs),
    _pixmap(pixmap),
    _spacing(spacing),
    _leading(leading),
    _scale(scale),
    _fade_duration(.2f)
{}

void font::update(float_t delta) {
  if (_animating) {
    _fade_time += delta;
    if (_fade_time >= _fade_duration) {
      _fade_time = _fade_duration;
      _animating = false;
    }
  }
}

void font::draw(const std::string& text, const geometry::point& position) const {
  if (text.empty()) {
    return;
  }

  const auto new_last = text.back();
  if (new_last != _last_char) {
    _last_char = new_last;
    _fade_time = 0.0f;
    _animating = true;
  }

  geometry::point cursor = position;
  const auto height = _glyphs.begin()->second.size().height() * _scale;

  for (size_t i = 0; i < text.size(); ++i) {
    char c = text[i];
    if (c == '\n') {
      cursor = geometry::point(position.x(), cursor.y() + height + _leading);
      continue;
    }

    const auto& glyph = _glyphs.at(static_cast<uint8_t>(c));
    auto size = glyph.size();

    uint8_t alpha = 255;
    if (_animating && i + 1 == text.size()) {
      float_t progress = _fade_time / _fade_duration;
      alpha = progress < 1.0f ? static_cast<uint8_t>(progress * 255) : 255;
    }

    _pixmap->draw(glyph, {cursor, size * _scale}, 0.0, reflection::none, alpha);
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
