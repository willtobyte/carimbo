#include "font.hpp"

using namespace graphics;

font::font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale)
    : _glyphs(glyphs), _pixmap(pixmap), _spacing(spacing), _leading(leading), _scale(scale) {}

void font::draw(const std::string &text, const geometry::point &position) const {
  geometry::point cursor = position;
  const auto height = _glyphs.begin()->second.size().height() * _scale;

  for (const char character : text) {
    switch (character) {
      case '\n':
        cursor = geometry::point(position.x(), cursor.y() + height + _leading);
        break;

      default: {
        const auto &glyph = _glyphs.at(static_cast<uint8_t>(character));
        auto size = glyph.size();
        _pixmap->draw(glyph, {cursor, size * _scale});
        cursor += std::make_pair('x', size.width() + _spacing);
        break;
      }
    }
  }
}
