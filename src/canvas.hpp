#pragma once

#include "common.hpp"

#include "renderer.hpp"

namespace graphics {
class canvas final {
public:
  explicit canvas(std::shared_ptr<renderer> renderer);
  ~canvas() noexcept = default;

  void set_pixels(const uint32_t* pixels) noexcept;

  void draw() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
};
}
