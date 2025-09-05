#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool)
  : _renderer(std::move(renderer))
  , _pixmappool(std::move(pixmappool))
{}

std::shared_ptr<font> fontfactory::get(const std::string& family) {
  std::filesystem::path p{family};
  const std::string filename = p.has_extension()
    ? family
    : std::format("fonts/{}.json", family);

  const auto [it, inserted] = _pool.try_emplace(filename);
  if (!inserted) [[unlikely]] {
    return it->second;
  }

  SDL_Texture* const origin = SDL_GetRenderTarget(*_renderer);

  try {
    std::println("[fontfactory] cache miss {}", filename);

    const auto& buffer = storage::io::read(filename);
    const auto& j = nlohmann::json::parse(buffer);

    const auto& glyphs = j["glyphs"].get_ref<const std::string&>();
    const auto spacing = j.value("spacing", int16_t{0});
    const auto leading = j.value("leading", int16_t{0});
    const auto scale   = j.value("scale",   float_t{1.0f});

    const auto pixmap = _pixmappool->get(std::format("blobs/overlay/{}.png", family));

    float_t fw, fh;
    if (!SDL_GetTextureSize(*pixmap, &fw, &fh)) [[unlikely]] {
      throw std::runtime_error(std::format("[SDL_GetTextureSize] {}", SDL_GetError()));
    }

    const int32_t tw = static_cast<int32_t>(fw);
    const int32_t th = static_cast<int32_t>(fh);

    std::unique_ptr<SDL_Texture, SDL_Deleter> target{
      SDL_CreateTexture(
        *_renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_TARGET,
        tw, th)
    };
    if (!target) [[unlikely]] {
      throw std::runtime_error(std::format("[SDL_CreateTexture] {}", SDL_GetError()));
    }

    SDL_SetRenderTarget(*_renderer, target.get());
    SDL_SetRenderDrawColor(*_renderer, 0, 0, 0, 0);
    SDL_RenderClear(*_renderer);
    SDL_RenderTexture(*_renderer, *pixmap, nullptr, nullptr);
    SDL_RenderPresent(*_renderer);

    std::unique_ptr<SDL_Surface, SDL_Deleter> surface{SDL_RenderReadPixels(*_renderer, nullptr)};
    if (!surface) [[unlikely]] {
      throw std::runtime_error(
        std::format("[SDL_RenderReadPixels] {}", SDL_GetError()));
    }

    SDL_SetRenderTarget(*_renderer, origin);

    const auto* pixels = static_cast<const uint32_t*>(surface->pixels);
    const auto separator = color(pixels[0]);

    glyphmap map;
    int32_t x = 0, y = 0;
    for (char glyph : glyphs) {
      while (x < tw && color(pixels[y * tw + x]) == separator) {
        ++x;
      }
      if (x >= tw) [[unlikely]] {
        throw std::runtime_error(std::format("missing glyph for '{}'", glyph));
      }

      int32_t w = 0;
      while (x + w < tw && color(pixels[y * tw + x + w]) != separator) {
        ++w;
      }

      int32_t h = 0;
      while (y + h < th && color(pixels[(y + h) * tw + x]) != separator) {
        ++h;
      }

      map[static_cast<uint8_t>(glyph)] = {
        { static_cast<float_t>(x), static_cast<float_t>(y) },
        { static_cast<float_t>(w), static_cast<float_t>(h) }
      };

      x += w;
    }

    it->second = std::make_shared<font>(
      glyphs, map, pixmap,
      spacing, leading, scale
    );

    // _loop();

    return it->second;
  } catch (...) {
    SDL_SetRenderTarget(*_renderer, origin);

    _pool.erase(it);
    throw;
  }
}

void fontfactory::flush() noexcept {
  std::println("[fontfactory] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](auto const& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });

  std::println("[fontfactory] {} objects have been flushed", count);
}

#ifdef DEBUG
void fontfactory::debug() const noexcept {
  std::println("fontfactory::debug total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  [{}] use_count={}", key, ptr.use_count());
  }
}
#endif

void fontfactory::set_loop(std::function<void()> fn) noexcept {
  _loop = std::move(fn);;
}
