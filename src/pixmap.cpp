#include "pixmap.hpp"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, const std::string &filename)
    : _renderer(std::move(renderer)) {
  // Load PNG data into 'output' and retrieve image 'size'
  std::vector<uint8_t> output;
  geometry::size size;
  std::tie(output, size) = _load_png(filename);

  // Create an SDL_Texture with the same dimensions as the image
  _texture = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(
        *_renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        size.width(),
        size.height()
      ),
      SDL_Deleter{}
  );
  if (!_texture) {
    throw std::runtime_error(fmt::format("[SDL_CreateTexture] Error creating texture: {}", SDL_GetError()));
  }

  constexpr auto pitch = size.width() * 4;
  if (SDL_UpdateTexture(_texture.get(), nullptr, output.data(), pitch) != 0) {
    throw std::runtime_error(fmt::format("[SDL_UpdateTexture] Error updating texture: {}", SDL_GetError()));
  }

  if (SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND) != 0) {
    throw std::runtime_error(fmt::format("[SDL_SetTextureBlendMode] Error setting blend mode: {}", SDL_GetError()));
  }
}

pixmap::pixmap(std::shared_ptr<renderer> renderer, std::unique_ptr<SDL_Surface, decltype(&SDL_DestroySurface)> surface)
    : _renderer(std::move(renderer)) {
  // Create an SDL_Texture from the provided SDL_Surface
  _texture = std::unique_ptr<SDL_Texture, decltype(&SDL_DestroyTexture)>(
      SDL_CreateTextureFromSurface(*_renderer, surface.get()),
      SDL_DestroyTexture);
  if (!_texture) {
    throw std::runtime_error(fmt::format("[SDL_CreateTextureFromSurface] Error creating texture from surface: {}", SDL_GetError()));
  }

  // Set the texture blend mode to blend
  if (SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND) != 0) {
    throw std::runtime_error(fmt::format("[SDL_SetTextureBlendMode] Error setting blend mode for texture: {}", SDL_GetError()));
  }
}

void pixmap::draw(
    const geometry::rectangle &source,
    const geometry::rectangle &destination,
    const double_t angle,
    reflection reflection,
    const uint8_t alpha
#ifdef HITBOX
    ,
    const std::optional<geometry::rectangle> &outline
#endif
) const noexcept {
  const SDL_FRect &src = source;
  const SDL_FRect &dst = destination;

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderTextureRotated(*_renderer, _texture.get(), &src, &dst, angle, nullptr, static_cast<SDL_FlipMode>(reflection));

#ifdef HITBOX
  if (outline) {
    const SDL_FRect &debug = *outline;

    SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
    SDL_RenderRect(*_renderer, &debug);
  }
#endif
}

pixmap::operator SDL_Texture *() const noexcept {
  return _texture.get();
}
