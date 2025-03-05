#include "canvas.hpp"

using namespace graphics;

canvas::canvas(std::shared_ptr<renderer> renderer)
    : _renderer{std::move(renderer)} {
  SDL_GetRendererOutputSize(*renderer, &_width, &_height);

  SDL_Texture *texture = SDL_CreateTexture(*_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, _width, _height);
  if (not texture) [[unlikely]] {
    const auto error = fmt::format("[SDL_CreateTexture] failed to create texture: {}", SDL_GetError());
    throw std::runtime_error(error);
  }

  _framebuffer.reset(texture);
}

void canvas::set_pixels(std::span<const uint32_t> pixels) noexcept {
  _pixels = std::move(pixels);
}

void canvas::draw() {
  if (_pixels.empty()) [[unlikely]] {
    return;
  }

  if (not _framebuffer) [[unlikely]] {
    throw std::runtime_error("[SDL_CreateTexture] framebuffer is null");
  }

  if (SDL_UpdateTexture(_framebuffer.get(), nullptr, _pixels.data(), _width * sizeof(uint32_t)) != 0) [[unlikely]] {
    throw std::runtime_error(fmt::format("[SDL_UpdateTexture] failed: {}", SDL_GetError()));
  }

  SDL_RenderCopy(*_renderer, _framebuffer.get(), nullptr, nullptr);
}
