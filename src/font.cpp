#include "font.hpp"

using namespace graphics;

font::font(
  std::string_view glyphs,
  const glyphmap& map,
  std::shared_ptr<pixmap> pixmap,
  int16_t spacing,
  int16_t leading,
  float scale
)
  : _glyphs(glyphs),
    _map(map),
    _pixmap(pixmap),
    _spacing(spacing),
    _leading(leading),
    _scale(scale)
{}

void font::draw(std::string_view text, const geometry::point& position, const std::weak_ptr<fonteffect>& effect) const {
  if (text.empty()) {
    return;
  }

  geometry::point cursor = position;

  const auto height = _map.begin()->second.height() * _scale;

  for (const auto ch : text) {
    if (ch == '\n') {
      cursor = geometry::point(position.x(), cursor.y() + height + _leading);
      continue;
    }

    const auto it = _map.find(ch);
    if (it == _map.end()) {
      continue;
    }

    const auto& glyph = it->second;
    const auto size = glyph.size();

    double angle = .0L;
    reflection reflection = reflection::none;
    uint8_t alpha = 255;
    float scale = 1.f;

    if (const auto e = effect.lock(); e) {
      angle = e->angle();
      reflection = e->reflection();
      alpha = e->alpha();
      scale = e->scale();
    }

    _pixmap->draw(
      glyph,
      {cursor, size * _scale * scale},
      angle,
      alpha,
      reflection
    );

    cursor += std::make_pair('x', size.width() + _spacing);
  }
}

std::string font::glyphs() const {
  return _glyphs;
}
