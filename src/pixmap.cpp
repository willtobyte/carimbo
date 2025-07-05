#include "pixmap.hpp"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, const std::string& filename)
    : _renderer(std::move(renderer)) {
  const auto buffer = storage::io::read(filename);

  const auto ctx = std::unique_ptr<spng_ctx, decltype(&spng_ctx_free)>(spng_ctx_new(0), spng_ctx_free);

  if (const auto error = spng_set_png_buffer(ctx.get(), buffer.data(), buffer.size()); error != SPNG_OK) [[unlikely]] {
    throw std::runtime_error(std::format("[spng_set_png_buffer] error while parsing image: {}, error: {}", filename, spng_strerror(error)));
  }

  spng_ihdr ihdr{};
  if (const auto error = spng_get_ihdr(ctx.get(), &ihdr); error != SPNG_OK) [[unlikely]] {
    throw std::runtime_error(std::format("[spng_get_ihdr] error while getting image information: {}, error: {}", filename, spng_strerror(error)));
  }

  const int format{SPNG_FMT_RGBA8};
  size_t length{0};
  if (const auto error = spng_decoded_image_size(ctx.get(), format, &length); error != SPNG_OK) [[unlikely]] {
    throw std::runtime_error(std::format("[spng_decoded_image_size] error while getting image size: {}, error: {}", filename, spng_strerror(error)));
  }

  std::vector<uint8_t> output(length);
  if (const auto error = spng_decode_image(ctx.get(), output.data(), length, format, SPNG_DECODE_TRNS); error != SPNG_OK) [[unlikely]] {
    throw std::runtime_error(std::format("[spng_decode_image] error while decoding image: {}, error: {}", filename, spng_strerror(error)));
  }

  _width = static_cast<int32_t>(ihdr.width);
  _height = static_cast<int32_t>(ihdr.height);
  _texture = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(
        *_renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STATIC,
        _width,
        _height
      ),
      SDL_Deleter{}
  );
  if (!_texture) {
    throw std::runtime_error(std::format("[SDL_CreateTexture] {}", SDL_GetError()));
  }

  const auto pitch = static_cast<int32_t>(ihdr.width * 4);
  if (!SDL_UpdateTexture(_texture.get(), nullptr, output.data(), pitch)) {
    throw std::runtime_error(std::format("[SDL_UpdateTexture] {}", SDL_GetError()));
  }

  if (!SDL_SetTextureBlendMode(_texture.get(), SDL_BLENDMODE_BLEND)) {
    throw std::runtime_error(std::format("[SDL_SetTextureBlendMode] {}", SDL_GetError()));
  }

  if (!SDL_SetTextureScaleMode(_texture.get(), SDL_SCALEMODE_NEAREST)) {
    throw std::runtime_error(std::format("[SDL_SetTextureScaleMode] {}", SDL_GetError()));
  }
}

void pixmap::draw(
    const geometry::rectangle& source,
    const geometry::rectangle& destination,
    const double_t angle,
    reflection reflection,
    const uint8_t alpha
#ifdef HITBOX
    ,
    const std::optional<geometry::rectangle>& outline
#endif
) const noexcept {
  const SDL_FRect& src = source;
  const SDL_FRect& dst = destination;

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderTextureRotated(*_renderer, _texture.get(), &src, &dst, angle, nullptr, static_cast<SDL_FlipMode>(reflection));

#ifdef HITBOX
  if (outline) {
    const SDL_FRect& debug = *outline;

    SDL_SetRenderDrawColor(*_renderer, 0, 255, 0, 255);
    SDL_RenderRect(*_renderer, &debug);
  }
#endif
}

pixmap::operator SDL_Texture *() const noexcept {
  return _texture.get();
}

int32_t pixmap::width() const noexcept {
  return _width;
}

int32_t pixmap::height() const noexcept {
  return _height;
}
