#include "font.hpp"

#include "label.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

font::font(
  std::string_view glyphs,
  const glyphmap& map,
  std::shared_ptr<pixmap> pixmap,
  std::shared_ptr<renderer> renderer,
  int16_t spacing,
  int16_t leading,
  float scale
)
  : _glyphs(glyphs),
    _map(map),
    _pixmap(std::move(pixmap)),
    _renderer(std::move(renderer)),
    _spacing(spacing),
    _leading(leading),
    _scale(scale)
{
  _vertices.reserve(256 * 4);
  _indices.reserve(256 * 6);

  const auto iw = 1.0f / static_cast<float>(_pixmap->width());
  const auto ih = 1.0f / static_cast<float>(_pixmap->height());

  for (size_t i = 0; i < 256; ++i) {
    if (const auto& g = _map[i]) {
      _uv_table[i] = {
        g->x * iw,
        g->y * ih,
        (g->x + g->w) * iw,
        (g->y + g->h) * ih
      };
    }
  }
}

void font::draw(std::string_view text, const vec2& position, const boost::unordered_flat_map<size_t, glyphprops>& effects) const {
  if (text.empty()) [[unlikely]] {
    return;
  }

  _vertices.clear();
  _indices.clear();

  const auto& first = _map[static_cast<uint8_t>(_glyphs[0])];
  assert(first && "first glyph must be valid");
  const auto height = first->h * _scale;

  auto cursor_x = position.x;
  auto cursor_y = position.y;

  auto i = 0uz;
  for (const auto ch : text) {
    if (ch == '\n') {
      cursor_x = position.x;
      cursor_y += height + _leading;
      ++i;
      continue;
    }

    const auto char_index = static_cast<uint8_t>(ch);
    const auto& glyph = _map[char_index];
    if (!glyph) {
      ++i;
      continue;
    }

    float xoffset = 0.f;
    float yoffset = 0.f;
    float scale = 1.f;
    uint8_t alpha = 255;

    if (const auto it = effects.find(i); it != effects.end()) {
      const auto& p = it->second;
      xoffset = p.xoffset;
      yoffset = p.yoffset;
      scale = p.scale;
      alpha = p.alpha;
    }

    const auto s = _scale * scale;
    const auto w = glyph->w * s;
    const auto h = glyph->h * s;
    const auto hw = w * 0.5f;
    const auto hh = h * 0.5f;

    const auto cx = cursor_x + xoffset + hw;
    const auto cy = cursor_y + yoffset + hh;

    const auto& uv = _uv_table[char_index];
    const SDL_FColor color{1.f, 1.f, 1.f, static_cast<float>(alpha) / 255.f};
    const auto base = static_cast<int32_t>(_vertices.size());

    _vertices.emplace_back(SDL_Vertex{{cx - hw, cy - hh}, color, {uv.u0, uv.v0}});
    _vertices.emplace_back(SDL_Vertex{{cx + hw, cy - hh}, color, {uv.u1, uv.v0}});
    _vertices.emplace_back(SDL_Vertex{{cx + hw, cy + hh}, color, {uv.u1, uv.v1}});
    _vertices.emplace_back(SDL_Vertex{{cx - hw, cy + hh}, color, {uv.u0, uv.v1}});

    _indices.emplace_back(base);
    _indices.emplace_back(base + 1);
    _indices.emplace_back(base + 2);
    _indices.emplace_back(base);
    _indices.emplace_back(base + 2);
    _indices.emplace_back(base + 3);

    cursor_x += glyph->w * _scale + _spacing;
    ++i;
  }

  if (_vertices.empty()) [[unlikely]] {
    return;
  }

  SDL_RenderGeometry(
    *_renderer,
    static_cast<SDL_Texture*>(*_pixmap),
    _vertices.data(),
    static_cast<int>(_vertices.size()),
    _indices.data(),
    static_cast<int>(_indices.size())
  );
}

std::string_view font::glyphs() const noexcept {
  return _glyphs;
}
