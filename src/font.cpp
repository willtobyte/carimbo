#include "font.hpp"

using namespace graphics;

font::font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, float_t scale)
    : _glyphs(glyphs), _pixmap(pixmap), _spacing(spacing), _scale(scale) {}

void font::draw(const std::string &text, const geometry::point &position) const noexcept {
  geometry::point cursor = position;

  for (const auto &character : text) {
    const auto &glyph = _glyphs.at(character);
    auto size = glyph.size();

    _pixmap->draw(glyph, {cursor, size * _scale});

    cursor += std::make_pair('x', size.width() + _spacing);
  }
}
