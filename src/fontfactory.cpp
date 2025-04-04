#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(std::shared_ptr<renderer> renderer) noexcept
    : _renderer(renderer) {}

std::shared_ptr<font> fontfactory::get(const std::string &family) {
  std::filesystem::path p{family};
  const auto key = p.has_extension() ? family : ("fonts/" + family + ".json");

  if (auto it = _pool.find(key); it != _pool.end()) {
    return it->second;
  }

  fmt::println("[fontfactory] cache miss {}", key);

  const auto &buffer = storage::io::read(key);
  const auto &j = nlohmann::json::parse(buffer);
  const auto &glyphs = j["glyphs"].get_ref<const std::string &>();
  const auto spacing = j.value("spacing", int16_t{0});
  const auto scale = j.value("scale", float_t{1.0f});

  std::vector<uint8_t> output;
  geometry::size size;
  std::tie(output, size) = _load_png(j["spritesheet"].get_ref<const std::string &>());

  const auto pitch = static_cast<int32_t>(size.width() * 4);
  auto surface = std::unique_ptr<SDL_Surface, SDL_Deleter>{
      SDL_CreateSurfaceFrom(
          size.width(),
          size.height(),
          SDL_PIXELFORMAT_ABGR8888,
          output.data(),
          pitch
      ),
      SDL_Deleter{}
  };

  // const auto format = SDL_GetPixelFormatDetails(surface->format);

  if (!surface) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateRGBSurfaceWithFormatFrom] error: {}", SDL_GetError()));
  }

  const auto pixels = static_cast<uint32_t *>(surface->pixels);
  const auto separator = color(pixels[0]);

  glyphmap map{};
  auto [x, y, w, h] = std::tuple{0, 0, 0, 0};
  const auto width = static_cast<int>(size.width());
  const auto height = static_cast<int>(size.height());

  for (const char glyph : glyphs) {
    while (x < width && color(pixels[y * width + x]) == separator) {
      ++x;
    }

    if (x >= width) [[unlikely]] {
      throw std::runtime_error(fmt::format("Error: missing glyph for '{}'", glyph));
    }

    w = 0;
    while (x + w < width && color(pixels[y * width + x + w]) != separator) {
      ++w;
    }

    h = 0;
    while (y + h < height && color(pixels[(y + h) * width + x]) != separator) {
      ++h;
    }

    // map[glyph] = {{x, y}, {w, h}};
    map[glyph] = {{static_cast<float_t>(x), static_cast<float_t>(y)}, {static_cast<float_t>(w), static_cast<float_t>(h)}};
    x += w;
  }

  auto ptr = std::make_shared<font>(
      map,
      std::make_shared<pixmap>(_renderer, std::move(surface)),
      spacing,
      scale
  );

  _pool.emplace(key, ptr);
  return ptr;
}

void fontfactory::flush() noexcept {
  const auto count = _pool.size();
  fmt::println("[fontfactory] actual size {}", count);
  _pool.clear();
  fmt::println("[fontfactory] {} objects have been flushed", count);
}
