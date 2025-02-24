#include "fontfactory.hpp"

using namespace graphics;

using json = nlohmann::json;

namespace {
constexpr std::string_view ext{".json"};
}

fontfactory::fontfactory(const std::shared_ptr<graphics::renderer> renderer) noexcept
    : _renderer(renderer) {}

std::shared_ptr<font> fontfactory::get(const std::string &family) {
  const auto name = family.ends_with(ext)
                        ? family.substr(0, family.size() - ext.size())
                        : family;

  if (auto it = _pool.find(name); it != _pool.end()) [[likely]] {
    return it->second;
  }

  std::cout << "[fontfactory] cache miss " << name << std::endl;

  const auto &buffer = storage::io::read("fonts/" + name + ".json");
  const auto &j = json::parse(buffer);
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

  glyphmap map;
  auto x = 0, y = 0, width = 0, height = 0;

  for (const char letter : alphabet) {
    while (x < size.width() && color(pixels[y * size.width() + x], surface->format) == separator) {
      ++x;
    }

    if (x >= size.width()) [[unlikely]] {
      std::ostringstream oss;
      oss << "Error: missing glyph for '" << letter << "'";
      throw std::runtime_error(oss.str());
    }

    width = 0;
    while (x + width < size.width() &&
           color(pixels[y * size.width() + x + width], surface->format) != separator) {
      ++width;
    }

    height = 0;
    while (y + height < size.height() &&
           color(pixels[(y + height) * size.width() + x], surface->format) != separator) {
      ++height;
    }

    map[letter] = {{x, y}, {width, height}};
    x += width;
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
  std::cout << "[fontfactory] actual size " << _pool.size() << std::endl;

  const auto count = std::erase_if(_pool, [](const auto &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::cout << "[fontfactory] " << count << " objects have been flushed" << std::endl;
}

void fontfactory::update(float_t delta) noexcept {
  UNUSED(delta);
}
