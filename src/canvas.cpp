#include "canvas.hpp"

using namespace graphics;

canvas::canvas(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {
  int32_t lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(*_renderer, &lw, &lh, &mode);

  float sx, sy;
  SDL_GetRenderScale(*_renderer, &sx, &sy);

  const auto width = static_cast<int32_t>(std::lround(static_cast<float>(lw) / sx));
  const auto height = static_cast<int32_t>(std::lround(static_cast<float>(lh) / sy));

  auto* texture = SDL_CreateTexture(*_renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture) [[unlikely]] {
    throw std::runtime_error(std::format("[SDL_CreateTexture] {}", SDL_GetError()));
  }

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  _framebuffer.reset(texture);

  const auto pixel = SDL_MapRGBA(SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGBA32), nullptr, 0, 0, 0, 0);
  const auto count = static_cast<size_t>(width) * static_cast<size_t>(height);

  _transparent = std::make_unique<uint32_t[]>(count);

  std::fill(_transparent.get(), _transparent.get() + count, pixel);
}

void canvas::set_pixels(const char* pixels) noexcept {
  if (!pixels) [[unlikely]] {
    return;
  }

  const auto ptr = _framebuffer.get();
  void* buffer = nullptr;
  auto pitch = 0;

  SDL_LockTexture(ptr, nullptr, &buffer, &pitch);
  std::memcpy(buffer, pixels, static_cast<size_t>(pitch) * static_cast<size_t>(ptr->h));
  SDL_UnlockTexture(ptr);
}

void canvas::clear() noexcept {
  set_pixels(reinterpret_cast<const char*>(_transparent.get()));
}

void canvas::draw() const noexcept {
  SDL_RenderTexture(*_renderer, _framebuffer.get(), nullptr, nullptr);
}
