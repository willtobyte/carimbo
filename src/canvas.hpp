#pragma once

#include "common.hpp"

#include "renderer.hpp"

namespace graphics {
class canvas {
public:
  explicit canvas(std::shared_ptr<renderer> renderer);
  ~canvas() noexcept = default;

  void set_pixels(const std::vector<uint32_t> &pixels) noexcept;

  void draw();

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
};
}
