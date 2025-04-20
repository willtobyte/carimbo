#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool)
  : _renderer(std::move(renderer))
  , _pixmappool(std::move(pixmappool))
{}

std::shared_ptr<font> fontfactory::get(const std::string &family) {
  std::filesystem::path p{family};
  const auto key = p.has_extension() ? family : ("fonts/" + family + ".json");

  if (auto it = _pool.find(key); it != _pool.end()) {
    return it->second;
  }

  fmt::println("[fontfactory] cache miss {}", key);

  const auto &buffer = storage::io::read(key);
  const auto &j = nlohmann::json::parse(buffer);
  const auto &glyphs  = j["glyphs"].get_ref<const std::string &>();
  const auto  spacing = j.value("spacing", int16_t{0});
  const auto  leading = j.value("leading", int16_t{0});
  const auto  scale   = j.value("scale", float_t{1.0f});

  const auto pixmap = _pixmappool->get(fmt::format("blobs/overlay/{}.png", family));

  float width, height;
  if (!SDL_GetTextureSize(*pixmap, &width, &height)) {
    panic("[SDL_GetTextureSize] failed to query texture size: {}", SDL_GetError());
  }

  std::unique_ptr<SDL_Texture, SDL_Deleter> target{
    SDL_CreateTexture(
      *_renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_TARGET,
      static_cast<int16_t>(width),
      static_cast<int16_t>(height)
    )
  };
  if (!target) {
    panic("[SDL_CreateTexture] failed to create texture: {}", SDL_GetError());
  }

  SDL_Texture *origin = SDL_GetRenderTarget(*_renderer);

  SDL_SetRenderTarget(*_renderer, target.get());
  SDL_SetRenderDrawColor(*_renderer, 0, 0, 0, 0);
  SDL_RenderClear(*_renderer);
  SDL_RenderTexture(*_renderer, *pixmap, nullptr, nullptr);
  SDL_RenderPresent(*_renderer);

  std::unique_ptr<SDL_Surface, SDL_Deleter> surface{SDL_RenderReadPixels(*_renderer, nullptr)};
  if (!surface) {
    panic("[SDL_RenderReadPixels] failed ti read pixels: {}", SDL_GetError());
  }

  SDL_SetRenderTarget(*_renderer, origin);

  auto *pixels = static_cast<uint32_t *>(surface->pixels);
  const auto separator = color(pixels[0]);

  glyphmap map;
  int32_t x = 0, y = 0, tw = static_cast<uint32_t>(width);
  for (char glyph : glyphs) {
    while (x < tw && color(pixels[y * tw + x]) == separator) {
      ++x;
    }

    if (x >= tw) [[unlikely]] {
      panic("error: missing glyph for '{}'", glyph);
    }

    int32_t w = 0;
    while (x + w < tw
           && color(pixels[y * tw + x + w]) != separator) {
      ++w;
    }

    int32_t h = 0;
    while (y + h < height
           && color(pixels[(y + h) * tw + x]) != separator) {
      ++h;
    }

    map[glyph] = {
      { static_cast<float_t>(x), static_cast<float_t>(y) },
      { static_cast<float_t>(w), static_cast<float_t>(h) }
    };

    x += w;
  }

  auto ptr = std::make_shared<font>(
    map,
    pixmap,
    spacing,
    leading,
    scale
  );

  _pool.emplace(key, ptr);

  return ptr;
}

void fontfactory::flush() {
  fmt::println("[fontfactory] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](auto const &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });

  fmt::println("[fontfactory] {} objects have been flushed", count);
}
