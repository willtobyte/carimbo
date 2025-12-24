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
  const auto iw = 1.0f / static_cast<float>(_pixmap->width());
  const auto ih = 1.0f / static_cast<float>(_pixmap->height());

  for (auto i = 0uz; i < 256; ++i) {
    if (const auto& g = _map[i]) {
      _uv_table[i] = {
        g->x * iw,
        g->y * ih,
        (g->x + g->w) * iw,
        (g->y + g->h) * ih
      };
    }
  }

  const auto& first = _map[static_cast<uint8_t>(_glyphs[0])];
  assert(first && "first glyph must be valid");
  _height = first->h * _scale;
}

void font::draw(std::string_view text, const vec2& position, const boost::unordered_flat_map<size_t, glyphprops>& effects) const {
  if (text.empty()) [[unlikely]] {
    return;
  }

  _vertices.clear();
  _indices.clear();

  constexpr auto inv = 1.f / 255.f;
  const auto line_height = _height + _leading;

  auto cursor_x = position.x;
  auto cursor_y = position.y;

  auto i = 0uz;
  for (const auto ch : text) {
    if (ch == '\n') {
      cursor_x = position.x;
      cursor_y += line_height;
      ++i;
      continue;
    }

    const auto char_index = static_cast<uint8_t>(ch);
    const auto& uv = _uv_table[char_index];
    if (uv.u0 == 0.f && uv.v0 == 0.f && uv.u1 == 0.f && uv.v1 == 0.f) [[unlikely]] {
      ++i;
      continue;
    }

    const auto& glyph = _map[char_index];
    const auto gw = glyph->w * _scale;
    const auto gh = glyph->h * _scale;
    const auto bhw = gw * 0.5f;
    const auto bhh = gh * 0.5f;

    auto hw = bhw;
    auto hh = bhh;
    auto cx = cursor_x + bhw;
    auto cy = cursor_y + bhh;
    SDL_FColor color{1.f, 1.f, 1.f, 1.f};

    if (const auto it = effects.find(i); it != effects.end()) {
      const auto& p = it->second;
      cx += p.xoffset;
      cy += p.yoffset;
      hw = bhw * p.scale;
      hh = bhh * p.scale;
      color = {p.r * inv, p.g * inv, p.b * inv, p.alpha * inv};
    }

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

    cursor_x += glyph->w + _spacing;
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
