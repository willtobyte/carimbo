#include "pixmap.hpp"
#include "deleters.hpp"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, const std::string &filename)
    : _renderer(std::move(renderer)) {
  std::vector<uint8_t> output;
  geometry::size size;
  std::tie(output, size) = _load_png(filename);

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
    panic("[SDL_CreateTexture] error creating texture: {}", SDL_GetError());
  }

  const auto pitch = static_cast<int32_t>(size.width() * 4);
  if (!SDL_UpdateTexture(_texture.get(), nullptr, output.data(), pitch)) {
    panic("[SDL_UpdateTexture] error updating texture: {}", SDL_GetError());
  }

  if (!SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND)) {
    panic("[SDL_SetTextureBlendMode] error setting blend mode: {}", SDL_GetError());
  }

  if (!SDL_SetTextureScaleMode(_texture.get(), SDL_SCALEMODE_NEAREST)) {
    panic("[SDL_SetTextureScaleMode] error setting texture scale mode: {}", SDL_GetError());
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
) const {
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

pixmap::~pixmap() {
  // TODO
  // auto* ptr = static_cast<std::pair<int, int>*>(SDL_GetTextureUserData(texture));
  // delete ptr;
  // SDL_SetTextureUserData(texture, nullptr);
  // SDL_DestroyTexture(texture);
}

pixmap::operator SDL_Texture *() const {
  return _texture.get();
}
