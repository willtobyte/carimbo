#include "canvas.hpp"

canvas::canvas() {
  int lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(renderer, &lw, &lh, &mode);

  float sx, sy;
  SDL_GetRenderScale(renderer, &sx, &sy);

  const auto width = static_cast<int>(std::lround(static_cast<float>(lw) / sx));
  const auto height = static_cast<int>(std::lround(static_cast<float>(lh) / sy));

  _framebuffer =
    std::unique_ptr<SDL_Texture, SDL_Deleter>(
      SDL_CreateTexture(renderer,
        SDL_PIXELFORMAT_RGBA32,
        SDL_TEXTUREACCESS_STREAMING,
        width, height));

  SDL_SetTextureBlendMode(_framebuffer.get(), SDL_BLENDMODE_BLEND);

  const auto pixel = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr, 0, 0, 0, 0);
  const auto count = static_cast<size_t>(width) * static_cast<size_t>(height);

  _transparent = std::make_unique<uint32_t[]>(count);

  std::fill(_transparent.get(), _transparent.get() + count, pixel);
}

void canvas::set_pixels(std::string_view pixels) noexcept {
  auto* const ptr = _framebuffer.get();
  auto* buffer = static_cast<void*>(nullptr);
  auto pitch = int{};

  SDL_LockTexture(ptr, nullptr, &buffer, &pitch);
  std::memcpy(buffer, pixels.data(), std::min(pixels.size(), static_cast<size_t>(pitch) * static_cast<size_t>(ptr->h)));
  SDL_UnlockTexture(ptr);
}

void canvas::clear() noexcept {
  set_pixels(reinterpret_cast<const char*>(_transparent.get()));
}

void canvas::draw() const noexcept {
  SDL_RenderTexture(renderer, _framebuffer.get(), nullptr, nullptr);
}
