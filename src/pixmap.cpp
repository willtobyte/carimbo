#include "pixmap.hpp"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, const std::string &filename)
    : _renderer(std::move(renderer)) {
  std::vector<uint8_t> output;
  geometry::size size;
  std::tie(output, size) = _load_png(filename);

  std::unique_ptr<SDL_Surface, SDL_Deleter> surface{
      SDL_CreateRGBSurfaceWithFormat(
          0,
          size.width(),
          size.height(),
          0,
          SDL_PIXELFORMAT_ABGR8888
      ),
      SDL_Deleter{}
  };
  if (!surface) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateRGBSurfaceWithFormat] error while creating surface, file: {}, error: {}", filename, SDL_GetError()));
  }

  std::memcpy(surface->pixels, output.data(), output.size());

  _texture = texture_ptr(SDL_CreateTextureFromSurface(*_renderer, surface.get()), SDL_Deleter{});
  if (!_texture) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateTextureFromSurface] error while creating texture from surface, file: {}", filename));
  }
}

pixmap::pixmap(std::shared_ptr<renderer> renderer, std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface)
    : _renderer(std::move(renderer)) {
  _texture = texture_ptr(SDL_CreateTextureFromSurface(*_renderer, surface.get()), SDL_Deleter{});
  if (!_texture) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateTextureFromSurface] error while creating texture, SDL Error: {}", SDL_GetError()));
  }
}

void pixmap::draw(
    const geometry::rect &source,
    const geometry::rect &destination,
    const double_t angle,
    reflection reflection,
    const uint8_t alpha
#ifdef HITBOX
    ,
    const std::optional<geometry::rect> &outline
#endif
) const noexcept {
  const SDL_Rect &src = source;
  const SDL_Rect &dst = destination;

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderCopyEx(*_renderer, _texture.get(), &src, &dst, angle, nullptr, static_cast<SDL_RendererFlip>(reflection));

#ifdef HITBOX
  if (outline) {
    const SDL_Rect &debug = *outline;

    SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
    SDL_RenderDrawRect(*_renderer, &debug);
  }
#endif
}

// geometry::size pixmap::size() const noexcept {
//   return _size;
// }

// void pixmap::set_size(const geometry::size &size) noexcept {
//   _size = size;
// }

pixmap::operator SDL_Texture *() const noexcept {
  return _texture.get();
}
