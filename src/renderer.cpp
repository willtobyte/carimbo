#include "renderer.hpp"

using namespace graphics;

renderer::renderer(SDL_Window *window)
    : _renderer(SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC), SDL_Deleter()) {
  if (!_renderer) [[unlikely]] {
    std::ostringstream oss;
    oss << "[SDL_CreateRenderer] failed to create renderer: " << SDL_GetError();
    throw std::runtime_error(oss.str());
  }
}

renderer::operator SDL_Renderer *() noexcept {
  return _renderer.get();
}

void renderer::begin() noexcept {
  SDL_RenderClear(*this);
}

void renderer::end() noexcept {
  SDL_RenderPresent(*this);
}

void renderer::draw(std::span<const uint32_t> pixels) {
  int width, height;
  SDL_GetRendererOutputSize(*this, &width, &height);

  SDL_Texture *framebuffer = SDL_CreateTexture(*this, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, width, height);

  if (!framebuffer) [[unlikely]] {
    std::ostringstream error;
    error << "Failed to create texture: " << SDL_GetError();
    throw std::runtime_error(error.str());
  }

  SDL_UpdateTexture(framebuffer, nullptr, pixels.data(), width * sizeof(uint32_t));
  SDL_RenderCopy(*this, framebuffer, nullptr, nullptr);
  SDL_DestroyTexture(framebuffer);
}
