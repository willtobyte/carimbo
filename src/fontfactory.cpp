#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool)
  : _renderer(std::move(renderer))
  , _pixmappool(std::move(pixmappool))
{}

std::shared_ptr<font> fontfactory::get(const std::string &family) {
  std::filesystem::path p{family};
  const auto filename = p.has_extension() ? family : ("fonts/" + family + ".json");

  if (auto it = _pool.find(filename); it != _pool.end()) {
    return it->second;
  }

  fmt::println("[fontfactory] cache miss {}", filename);

  const auto &buffer = storage::io::read(filename);
  const auto &j = nlohmann::json::parse(buffer);

  const auto &glyphs  = j["glyphs"].get_ref<const std::string &>();
  const auto  spacing = j.value("spacing", int16_t{0});
  const auto  leading = j.value("leading", int16_t{0});
  const auto  scale   = j.value("scale", float_t{1.0f});

  const auto pixmap = _pixmappool->get(fmt::format("blobs/overlay/{}.png", family));

  float_t width, height;
  if (!SDL_GetTextureSize(*pixmap, &width, &height)) {
    throw std::runtime_error(fmt::format("[SDL_GetTextureSize] {}", SDL_GetError()));
  }

  std::unique_ptr<SDL_Texture, SDL_Deleter> target{
    SDL_CreateTexture(
      *_renderer,
      SDL_PIXELFORMAT_ARGB8888,
      SDL_TEXTUREACCESS_TARGET,
      static_cast<int32_t>(width),
      static_cast<int32_t>(height)
    )
  };
  if (!target) {
    throw std::runtime_error(fmt::format("[SDL_CreateTexture] {}", SDL_GetError()));
  }

  SDL_Texture *origin = SDL_GetRenderTarget(*_renderer);

  SDL_SetRenderTarget(*_renderer, target.get());
  SDL_SetRenderDrawColor(*_renderer, 0, 0, 0, 0);
  SDL_RenderClear(*_renderer);
  SDL_RenderTexture(*_renderer, *pixmap, nullptr, nullptr);
  SDL_RenderPresent(*_renderer);

  std::unique_ptr<SDL_Surface, SDL_Deleter> surface{SDL_RenderReadPixels(*_renderer, nullptr)};
  if (!surface) {
    throw std::runtime_error(fmt::format("[SDL_RenderReadPixels] {}", SDL_GetError()));
  }

  SDL_SetRenderTarget(*_renderer, origin);

  auto *pixels = static_cast<uint32_t *>(surface->pixels);

  const auto separator = color(pixels[0]);

  glyphmap map;
  auto [x, y, tw, th] = std::tuple{0, 0, int32_t(width), int32_t(height)};
  for (char glyph : glyphs) {
    while (x < tw && color(pixels[y * tw + x]) == separator) {
      ++x;
    }

    if (x >= tw) [[unlikely]] {
      throw std::runtime_error(fmt::format("missing glyph for '{}'", glyph));
    }

    int32_t w = 0;
    while (x + w < tw
           && color(pixels[y * tw + x + w]) != separator) {
      ++w;
    }

    int32_t h = 0;
    while (y + h < th
           && color(pixels[(y + h) * tw + x]) != separator) {
      ++h;
    }

    map[static_cast<uint8_t>(glyph)] = {
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

  _pool.emplace(filename, ptr);

  return ptr;
}

void fontfactory::flush() {
  fmt::println("[fontfactory] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](auto const &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });

  fmt::println("[fontfactory] {} objects have been flushed", count);
}
