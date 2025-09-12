#include "font.hpp"

using namespace graphics;

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

    double_t angle = .0L;
    reflection reflection = reflection::none;
    uint8_t alpha = 255;
    // float_t scale = 1.f;

    if (const auto e = effect.lock()) {
      angle = e->angle();
      reflection = e->reflection();
      alpha = e->alpha();
      // scale = e->scale();
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
