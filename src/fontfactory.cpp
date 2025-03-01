#include "fontfactory.hpp"

using namespace graphics;

fontfactory::fontfactory(const std::shared_ptr<graphics::renderer> renderer) noexcept
    : _renderer(renderer) {}

std::shared_ptr<font> fontfactory::get(const std::string &family) {
  std::cout << "[fontfactory] cache miss " << family << std::endl;

  const auto &buffer = storage::io::read("fonts/" + family + ".json");
  const auto &j = nlohmann::json::parse(buffer);
  const auto &alphabet = j["alphabet"].get<std::string>();
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
    std::ostringstream oss;
    oss << "[SDL_CreateRGBSurfaceWithFormatFrom] error: " << SDL_GetError();
    throw std::runtime_error(oss.str());
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
      std::ostringstream oss;
      oss << "Error: missing glyph for '" << letter << "'";
      throw std::runtime_error(oss.str());
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

  _pool.emplace(family, ptr);
  return ptr;
}

void fontfactory::flush() noexcept {
  const auto count = _pool.size();
  std::cout << "[fontfactory] actual size " << count << std::endl;
  _pool.clear();
  std::cout << "[fontfactory] " << count << " objects have been flushed" << std::endl;
}
