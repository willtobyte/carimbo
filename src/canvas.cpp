#include "canvas.hpp"

using namespace graphics;

canvas::canvas(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {

  int32_t lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(*_renderer, &lw, &lh, &mode);

  float_t sx, sy;
  SDL_GetRenderScale(*_renderer, &sx, &sy);

  const auto width = static_cast<int32_t>(lw / sx);
  const auto height = static_cast<int32_t>(lh / sy);

  SDL_Texture *texture = SDL_CreateTexture(*_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_CreateTexture] failed to create texture: {}", SDL_GetError()));
  }

  SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);

  _framebuffer.reset(texture);
}

void canvas::set_pixels(const std::vector<uint32_t> &pixels) noexcept {
  void *ptr = nullptr;
  auto pitch = 0;
  if (!SDL_LockTexture(_framebuffer.get(), nullptr, &ptr, &pitch)) [[unlikely]] {
    return;
  }

  using value_type = std::remove_reference_t<decltype(pixels)>::value_type;
  std::memcpy(ptr, pixels.data(), pixels.size() * sizeof(value_type));

  SDL_UnlockTexture(_framebuffer.get());
}

void canvas::draw() {
  if (!_framebuffer) [[unlikely]] {
    throw std::runtime_error("[SDL_CreateTexture] framebuffer is null");
  }

  SDL_RenderTexture(*_renderer, _framebuffer.get(), nullptr, nullptr);
}
