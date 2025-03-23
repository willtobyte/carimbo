#include "canvas.hpp"

using namespace graphics;

canvas::canvas(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {

  int32_t logocal_width, logical_height;
  SDL_RenderGetLogicalSize(*_renderer, &logocal_width, &logical_height);

  float_t scale_x, scale_y;
  SDL_RenderGetScale(*_renderer, &scale_x, &scale_y);

  const auto width = static_cast<int32_t>(logocal_width / scale_x);
  const auto height = static_cast<int32_t>(logical_height / scale_y);

  SDL_Texture *texture = SDL_CreateTexture(*_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);
  if (!texture) [[unlikely]] {
    const auto error = fmt::format("[SDL_CreateTexture] failed to create texture: {}", SDL_GetError());
    throw std::runtime_error(error);
  }

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
