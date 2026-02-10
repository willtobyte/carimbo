#include "font.hpp"

#include "io.hpp"
#include "label.hpp"
#include "pixmap.hpp"

font::font(std::string_view family) {
  auto json = unmarshal::parse(io::read(std::format("fonts/{}.json", family)));

  _glyphs = json["glyphs"].get<std::string_view>();
  _spacing = json["spacing"].get<int16_t>();
  _leading = json["leading"].get<int16_t>();
  const auto scale = json["scale"].get(1.f);

  _pixmap = std::make_shared<pixmap>(std::format("blobs/overlay/{}.png", family));
  const auto width = _pixmap->width();
  const auto height = _pixmap->height();

  const auto target = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(
          renderer,
          SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET,
          width, height));

  auto* const origin = SDL_GetRenderTarget(renderer);

  SDL_FlushRenderer(renderer);

  SDL_FRect destination{0, 0, static_cast<float>(width), static_cast<float>(height)};

  SDL_SetRenderTarget(renderer, target.get());
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
  SDL_RenderClear(renderer);
  SDL_RenderTexture(renderer, *_pixmap, nullptr, &destination);

  const auto surface = std::unique_ptr<SDL_Surface, SDL_Deleter>(SDL_RenderReadPixels(renderer, nullptr));

  SDL_SetRenderTarget(renderer, origin);

  const auto* pixels = static_cast<const uint32_t*>(surface->pixels);
  const auto separator = pixels[0];

  const auto iw = 1.0f / static_cast<float>(width);
  const auto ih = 1.0f / static_cast<float>(height);

  constexpr auto inset = .5f;

  auto x = 0, y = 0;
  auto first = true;
  for (char glyph : _glyphs) {
    while (x < width && pixels[y * width + x] == separator) {
      ++x;
    }

    assert(x < width && std::format("missing glyph for '{}'", glyph).c_str());

    auto w = 0;
    while (x + w < width && pixels[y * width + x + w] != separator) {
      ++w;
    }

    auto h = 0;
    while (y + h < height && pixels[(y + h) * width + x] != separator) {
      ++h;
    }

    const auto fx = static_cast<float>(x);
    const auto fy = static_cast<float>(y);
    const auto fw = static_cast<float>(w);
    const auto fh = static_cast<float>(h);

    _props[static_cast<uint8_t>(glyph)] = {
      (fx + inset) * iw,
      (fy + inset) * ih,
      (fx + fw - inset) * iw,
      (fy + fh - inset) * ih,
      fw * scale,
      fh * scale,
      fw,
      true
    };

    if (first) {
      _fontheight = fh * scale;
      first = false;
    }

    x += w;
  }
}

void font::draw(std::string_view text, const vec2& position, const boost::unordered_flat_map<size_t, glypheffect>& effects) const {
  if (text.empty()) [[unlikely]] {
    return;
  }

  _vertices.clear();
  _indices.clear();

  _vertices.reserve(text.size() * 4);
  _indices.reserve(text.size() * 6);

  constexpr auto inv = 1.f / 255.f;

  auto cx = position.x;
  auto cy = position.y;

  auto i = 0uz;
  for (const auto ch : text) {
    if (ch == '\n') [[unlikely]] {
      cx = position.x;
      cy += _fontheight + _leading;
      ++i;
      continue;
    }

    const auto& glyph = _props[static_cast<uint8_t>(ch)];
    if (!glyph.valid) [[unlikely]] {
      ++i;
      continue;
    }

    const auto bhw = glyph.sw * .5f;
    const auto bhh = glyph.sh * .5f;

    auto hw = bhw;
    auto hh = bhh;
    auto gx = cx + bhw;
    auto gy = cy + bhh;
    SDL_FColor color{1.f, 1.f, 1.f, 1.f};

    if (const auto it = effects.find(i); it != effects.end()) {
      const auto& e = it->second;
      gx += e.xoffset;
      gy += e.yoffset;
      hw = bhw * e.scale;
      hh = bhh * e.scale;
      color = {e.r * inv, e.g * inv, e.b * inv, e.alpha * inv};
    }

    const auto base = static_cast<int32_t>(_vertices.size());

    _vertices.emplace_back(SDL_Vertex{{gx - hw, gy - hh}, color, {glyph.u0, glyph.v0}});
    _vertices.emplace_back(SDL_Vertex{{gx + hw, gy - hh}, color, {glyph.u1, glyph.v0}});
    _vertices.emplace_back(SDL_Vertex{{gx + hw, gy + hh}, color, {glyph.u1, glyph.v1}});
    _vertices.emplace_back(SDL_Vertex{{gx - hw, gy + hh}, color, {glyph.u0, glyph.v1}});

    _indices.emplace_back(base);
    _indices.emplace_back(base + 1);
    _indices.emplace_back(base + 2);
    _indices.emplace_back(base);
    _indices.emplace_back(base + 2);
    _indices.emplace_back(base + 3);

    cx += glyph.w + _spacing;
    ++i;
  }

  if (_vertices.empty()) [[unlikely]] {
    return;
  }

  SDL_RenderGeometry(
    renderer,
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
