#pragma once

#include "common.hpp"

namespace graphics {
class canvas {
public:
  explicit canvas(std::shared_ptr<renderer> renderer);
  ~canvas() noexcept = default;

  void set_pixels(std::span<const uint32_t> pixels) noexcept;

  void draw() const;

private:
  int32_t _width, _height;
  std::span<const uint32_t> _pixels;
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
};
}
