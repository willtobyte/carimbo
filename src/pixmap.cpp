#include "pixmap.hpp"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_PNG
#include "stb_image.h"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, std::string_view filename)
    : _renderer(std::move(renderer)) {
  const auto buffer = storage::io::read(filename);

  int width, height, channels;
  const auto pixels = unwrap(
    std::unique_ptr<stbi_uc, STBI_Deleter>(
      stbi_load_from_memory(
        buffer.data(),
        static_cast<int>(buffer.size()),
        &width,
        &height,
        &channels,
        STBI_rgb_alpha
      )
    ),
    std::format("error while loading image: {}", filename)
  );

  _width = width;
  _height = height;
  _texture = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(
        *_renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        _width, _height));

  const auto pitch = width * 4;
  SDL_UpdateTexture(_texture.get(), nullptr, pixels.get(), pitch);
  SDL_SetTextureScaleMode(_texture.get(), SDL_SCALEMODE_NEAREST);
}

void pixmap::draw(
    const float sx, const float sy, const float sw, const float sh,
    const float dx, const float dy, const float dw, const float dh,
    const double angle,
    const uint8_t alpha,
    const reflection reflection
) const {
  const SDL_FRect source{ sx, sy, sw, sh };
  const SDL_FRect destination{ dx, dy, dw, dh };

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderTextureRotated(*_renderer, _texture.get(), &source, &destination, angle, nullptr, static_cast<SDL_FlipMode>(reflection));
}

pixmap::operator SDL_Texture*() const {
  return _texture.get();
}

int pixmap::width() const {
  return _width;
}

int pixmap::height() const {
  return _height;
}
