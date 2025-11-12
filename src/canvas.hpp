#pragma once

#include "common.hpp"

namespace graphics {
class renderer;
class canvas final {
public:
  explicit canvas(std::shared_ptr<renderer> renderer);
  ~canvas() = default;

  void set_pixels(const char* pixels);

  void clear();

  void draw() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
  std::unique_ptr<uint32_t[]> _transparent;
};
}
