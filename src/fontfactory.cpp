#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(const std::shared_ptr<graphics::renderer> renderer) noexcept
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
  const auto &alphabet = j["alphabet"].get_ref<const std::string &>();
  const auto spacing = j["spacing"].get<int16_t>();
  const auto scale = j["scale"].get<float_t>();

  std::vector<uint8_t> output;
  geometry::size size;
  std::tie(output, size) = _load_png(j["spritesheet"].get_ref<const std::string &>());

  auto surface = std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)>{
      SDL_CreateRGBSurfaceWithFormatFrom(
          output.data(),
          size.width(),
          size.height(),
          32,
          size.width() * 4,
          SDL_PIXELFORMAT_ABGR8888
      ),
      SDL_FreeSurface
  };

  if (!surface) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateRGBSurfaceWithFormatFrom] error: {}", SDL_GetError()));
  }

  const auto pixels = static_cast<uint32_t *>(surface->pixels);
  const auto separator = color(pixels[0], surface->format);

  glyphmap map{};
  auto [x, y, w, h] = std::tuple{0, 0, 0, 0};
  const auto width = size.width();
  const auto height = size.height();

  for (const char letter : alphabet) {
    while (x < width && color(pixels[y * width + x], surface->format) == separator) {
      ++x;
    }

    if (x >= width) [[unlikely]] {
      throw std::runtime_error(fmt::format("Error: missing glyph for '{}'", letter));
    }

    w = 0;
    while (x + w < width &&
           color(pixels[y * width + x + w], surface->format) != separator) {
      ++w;
    }

    h = 0;
    while (y + h < height &&
           color(pixels[(y + h) * width + x], surface->format) != separator) {
      ++h;
    }

    map[letter] = {{x, y}, {w, h}};
    x += w;
  }

  auto ptr = std::make_shared<font>(
      std::move(map),
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
