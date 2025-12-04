#include "fontfactory.hpp"

#include "font.hpp"
#include "io.hpp"
#include "pixmappool.hpp"
#include "renderer.hpp"

fontfactory::fontfactory(std::shared_ptr<renderer> renderer, std::shared_ptr<pixmappool> pixmappool)
  : _renderer(std::move(renderer))
  , _pixmappool(std::move(pixmappool))
{}

std::shared_ptr<font> fontfactory::get(std::string_view family) noexcept {
  std::filesystem::path p(family);
  const std::string filename = p.has_extension()
    ? std::string{family}
    : std::format("fonts/{}.json", family);

  const auto [it, inserted] = _pool.try_emplace(filename);
  if (inserted) {
    std::println("[fontfactory] cache miss {}", filename);

    const auto buffer = io::read(filename);
    const auto j = nlohmann::json::parse(buffer);

    const auto glyphs = j["glyphs"].get<std::string_view>();
    const auto spacing = j.value("spacing", int16_t{0});
    const auto leading = j.value("leading", int16_t{0});
    const auto scale   = j.value("scale",   float{1.f});

    const auto pixmap = _pixmappool->get(std::format("blobs/overlay/{}.png", family));
    const auto width = pixmap->width();
    const auto height = pixmap->height();

    const auto target = std::unique_ptr<SDL_Texture, SDL_Deleter>(
        SDL_CreateTexture(
          *_renderer,
          SDL_PIXELFORMAT_RGBA32,
          SDL_TEXTUREACCESS_TARGET,
          width, height));

    auto* const origin = SDL_GetRenderTarget(*_renderer);

    SDL_FlushRenderer(*_renderer);

    SDL_FRect destination{0, 0, static_cast<float>(width), static_cast<float>(height)};

    SDL_SetRenderTarget(*_renderer, target.get());
    SDL_SetRenderDrawColor(*_renderer, 0, 0, 0, 0);
    SDL_RenderClear(*_renderer);
    SDL_RenderTexture(*_renderer, *pixmap, nullptr, &destination);

    const auto surface = std::unique_ptr<SDL_Surface, SDL_Deleter>(SDL_RenderReadPixels(*_renderer, nullptr));

    SDL_SetRenderTarget(*_renderer, origin);

    const auto* pixels = static_cast<const uint32_t*>(surface->pixels);
    const auto separator = pixels[0];

    glyphmap map;
    auto x = 0, y = 0;
    for (char glyph : glyphs) {
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

      map[static_cast<uint8_t>(glyph)] ={
        static_cast<float>(x), static_cast<float>(y),
        static_cast<float>(w), static_cast<float>(h)
      };

      x += w;
    }

    return it->second = std::make_shared<font>(
      glyphs,
      map,
      pixmap,
      spacing,
      leading,
      scale
    );
  }

  return it->second;
}

void fontfactory::flush() {
  std::println("[fontfactory] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](auto const& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });

  std::println("[fontfactory] {} objects have been flushed", count);
}

#ifndef NDEBUG
void fontfactory::debug() const {
  std::println("[fontfactory.debug] total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  {} use_count={}", key, ptr.use_count());
  }
}
#endif
