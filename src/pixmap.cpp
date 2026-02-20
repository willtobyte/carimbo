#include "pixmap.hpp"

pixmap::pixmap(std::string_view filename) {
  const auto buffer = io::read(filename);

  auto spng =
    std::unique_ptr<spng_ctx, SPNG_Deleter>(spng_ctx_new(SPNG_CTX_IGNORE_ADLER32));

  spng_set_crc_action(spng.get(), SPNG_CRC_USE, SPNG_CRC_USE);
  spng_set_png_buffer(spng.get(), buffer.data(), buffer.size());

  spng_ihdr ihdr;
  spng_get_ihdr(spng.get(), &ihdr);

  _width = static_cast<int>(ihdr.width);
  _height = static_cast<int>(ihdr.height);

  size_t length;
  spng_decoded_image_size(spng.get(), SPNG_FMT_RGBA8, &length);

  auto pixels = std::make_unique_for_overwrite<uint8_t[]>(length);
  spng_decode_image(spng.get(), pixels.get(), length, SPNG_FMT_RGBA8, SPNG_DECODE_TRNS);

  _texture = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, _width, _height));

  SDL_UpdateTexture(_texture.get(), nullptr, pixels.get(), _width * SDL_BYTESPERPIXEL(SDL_PIXELFORMAT_RGBA32));
  SDL_SetTextureScaleMode(_texture.get(), SDL_SCALEMODE_NEAREST);
  SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND);
}

void pixmap::draw(
    const float sx, const float sy, const float sw, const float sh,
    const float dx, const float dy, const float dw, const float dh,
    const double angle,
    const uint8_t alpha,
    const flip flip
) const noexcept {
  const SDL_FRect source{ sx, sy, sw, sh };
  const SDL_FRect destination{ dx, dy, dw, dh };

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderTextureRotated(renderer, _texture.get(), &source, &destination, angle, nullptr, static_cast<SDL_FlipMode>(flip));
}

pixmap::operator SDL_Texture*() const noexcept {
  return _texture.get();
}

int pixmap::width() const noexcept {
  return _width;
}

int pixmap::height() const noexcept {
  return _height;
}
