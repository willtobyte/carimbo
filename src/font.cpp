#include "font.hpp"

#include "fonteffect.hpp"
#include "pixmap.hpp"

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

void font::draw(std::string_view text, const vec2& position, const std::weak_ptr<fonteffect>& effect) const {
  if (text.empty()) {
    return;
  }

  vec2 cursor = position;

  const auto& first = _map[static_cast<uint8_t>(_glyphs[0])];
  assert(first && "first glyph must be valid");
  const float height = first->h * _scale;

  for (const auto ch : text) {
    if (ch == '\n') {
      cursor = vec2(position.x, cursor.y + height + _leading);
      continue;
    }

    const auto index = static_cast<uint8_t>(ch);
    const auto& glyph = _map[index];
    if (!glyph) {
      continue;
    }

    double angle = .0L;
    flip flip = flip::none;
    uint8_t alpha = 255;
    float scale = 1.f;

    if (const auto e = effect.lock(); e) {
      angle = e->angle();
      flip = e->flip();
      alpha = e->alpha();
      scale = e->scale();
    }

    const auto s = _scale * scale;

    _pixmap->draw(
      glyph->x, glyph->y, glyph->w, glyph->h,
      cursor.x, cursor.y, glyph->w * s, glyph->h * s,
      angle,
      alpha,
      flip
    );

    cursor.x += glyph->w + _spacing;
  }
}

std::string_view font::glyphs() const noexcept {
  return _glyphs;
}
