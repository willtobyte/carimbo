#include "canvas.hpp"

using namespace graphics;

canvas::canvas(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {

  int32_t lw, lh;
  SDL_RenderGetLogicalSize(*_renderer, &lw, &lh);

  float_t sx, sy;
  SDL_RenderGetScale(*_renderer, &sx, &sy);

  const auto width = static_cast<int32_t>(lw / sx);
  const auto height = static_cast<int32_t>(lh / sy);

  SDL_Texture *texture = SDL_CreateTexture(*_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture) [[unlikely]] {
    const auto error = fmt::format("[SDL_CreateTexture] failed to create texture: {}", SDL_GetError());
    throw std::runtime_error(error);
  }

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  ///
  void *ptr = nullptr;
  int pitch = 0;
  if (SDL_LockTexture(texture, nullptr, &ptr, &pitch) != 0) [[unlikely]] {
    SDL_DestroyTexture(texture);
    throw std::runtime_error("[SDL_LockTexture] failed to lock texture");
  }

  auto *pixels = static_cast<uint32_t *>(ptr);
  const uint32_t green = 0xFF00FF00;
  const uint32_t transparent = 0xFF000000;

  for (int32_t y = 0; y < height; ++y) {
    for (int32_t x = 0; x < width; ++x) {
      const bool border = x < 4 || x >= width - 4 || y < 4 || y >= height - 4;
      pixels[y * width + x] = border ? green : transparent;
    }
  }

  SDL_UnlockTexture(texture);
  ///

  _framebuffer.reset(texture);
}

void canvas::set_pixels(const std::vector<uint32_t> &pixels) noexcept {
  void *ptr = nullptr;
  int pitch = 0;
  if (SDL_LockTexture(_framebuffer.get(), nullptr, &ptr, &pitch) != 0) [[unlikely]] {
    return;
  }

  std::memcpy(ptr, pixels.data(), pixels.size() * sizeof(uint32_t));

  SDL_UnlockTexture(_framebuffer.get());
}

void canvas::draw() {
  if (!_framebuffer) [[unlikely]] {
    throw std::runtime_error("[SDL_CreateTexture] framebuffer is null");
  }

  // SDL_RenderCopy(*_renderer, _framebuffer.get(), nullptr, nullptr);
}
