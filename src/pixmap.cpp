#include "pixmap.hpp"

using namespace graphics;

pixmap::pixmap(const std::shared_ptr<renderer> &renderer, const std::string &filename)
    : _renderer(renderer), _size(0, 0) {
  std::vector<uint8_t> output;
  std::tie(output, _size) = _load_png(filename);

  std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface{
      SDL_CreateRGBSurfaceWithFormat(
          0,
          _size.width(),
          _size.height(),
          0,
          SDL_PIXELFORMAT_ABGR8888
      ),
      SDL_FreeSurface
  };
  if (!surface) [[unlikely]] {
    std::ostringstream oss;
    oss << "[SDL_CreateRGBSurfaceWithFormat] error while creating surface, file: "
        << filename << ", error: " << SDL_GetError();
    throw std::runtime_error(oss.str());
  }

  std::memcpy(surface->pixels, output.data(), output.size());

  _texture = texture_ptr(SDL_CreateTextureFromSurface(*renderer, surface.get()), SDL_Deleter());
  if (!_texture) [[unlikely]] {
    std::ostringstream oss;
    oss << "[SDL_CreateTextureFromSurface] error while creating texture from surface, file: "
        << filename;
    throw std::runtime_error(oss.str());
  }
}

pixmap::pixmap(const std::shared_ptr<renderer> &renderer, std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface)
    : _renderer(renderer), _size(surface->w, surface->h) {

  _texture = texture_ptr(SDL_CreateTextureFromSurface(*renderer, surface.get()), SDL_Deleter());
  if (!_texture) [[unlikely]] {
    std::ostringstream oss;
    oss << "[SDL_CreateTextureFromSurface] error while creating texture, SDL Error: "
        << SDL_GetError();
    throw std::runtime_error(oss.str());
  }
}

void pixmap::draw(
    const geometry::rect &source,
    const geometry::rect &destination,
    const double_t angle,
    reflection reflection,
    const uint8_t alpha
#ifdef DEBUG
    ,
    const geometry::rect &outline
#endif
) const noexcept {
  const SDL_Rect &src = source;
  const SDL_Rect &dst = destination;

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderCopyEx(*_renderer, _texture.get(), &src, &dst, angle, nullptr, static_cast<SDL_RendererFlip>(reflection));

#ifdef DEBUG
  const SDL_Rect &debug = outline;

  SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
  SDL_RenderDrawRect(*_renderer, &debug);
#endif
}

geometry::size pixmap::size() const noexcept {
  return _size;
}

void pixmap::set_size(const geometry::size &size) noexcept {
  _size = size;
}

pixmap::operator SDL_Texture *() const noexcept {
  return _texture.get();
}
