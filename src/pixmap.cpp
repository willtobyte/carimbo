#include "pixmap.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

using namespace graphics;

pixmap::pixmap(std::shared_ptr<renderer> renderer, std::string_view filename)
    : _renderer(std::move(renderer)) {
  const auto buffer = storage::io::read(filename);

  int32_t width, height, channels;
  const auto pixels = std::unique_ptr<stbi_uc, STBI_Deleter>(
    stbi_load_from_memory(
      buffer.data(),
      static_cast<int>(buffer.size()),
      &width,
      &height,
      &channels,
      STBI_rgb_alpha
    )
  );

  if (!pixels) [[unlikely]] {
    throw std::runtime_error(
      std::format("[stbi_load_from_memory] error while loading image: {}, error: {}",
        filename,
        stbi_failure_reason()));
  }

  _width = width;
  _height = height;
  _texture = std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(
        *_renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STATIC,
        _width,
        _height
      ),
      SDL_Deleter{}
  );
  if (!_texture) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_CreateTexture] {}", SDL_GetError()));
  }

  const auto pitch = static_cast<int32_t>(width * 4);
  if (!SDL_UpdateTexture(_texture.get(), nullptr, pixels.get(), pitch)) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_UpdateTexture] {}", SDL_GetError()));
  }

  if (!SDL_SetTextureScaleMode(_texture.get(), SDL_SCALEMODE_NEAREST)) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_SetTextureScaleMode] {}", SDL_GetError()));
  }
}

void pixmap::draw(
    const geometry::point& source,
    const geometry::rectangle& destination,
    const double angle,
    const uint8_t alpha,
    reflection reflection
) const {
  const geometry::rectangle _source{
    source.x(),
    source.y(),
    static_cast<float>(width()),
    static_cast<float>(height())
  };

  draw(
    _source,
    destination,
    angle,
    alpha,
    reflection
  );
}

void pixmap::draw(
    const geometry::rectangle& source,
    const geometry::rectangle& destination,
    const double angle,
    const uint8_t alpha,
    reflection reflection
) const {
  const SDL_FRect& _source = source;
  const SDL_FRect& _destination = destination;

  SDL_SetTextureAlphaMod(_texture.get(), alpha);
  SDL_RenderTextureRotated(*_renderer, _texture.get(), &_source, &_destination, angle, nullptr, static_cast<SDL_FlipMode>(reflection));
}

pixmap::operator SDL_Texture* () const {
  return _texture.get();
}

int32_t pixmap::width() const {
  return _width;
}

int32_t pixmap::height() const {
  return _height;
}

void pixmap::set_blendmode(blendmode mode) {
  if (!SDL_SetTextureBlendMode(_texture.get(), static_cast<SDL_BlendMode>(mode))) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_SetTextureBlendMode] {}", SDL_GetError()));
  }
}
