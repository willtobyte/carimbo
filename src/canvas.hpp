#pragma once

#include "common.hpp"

namespace graphics {
class renderer;
class canvas final {
public:
  explicit canvas(std::shared_ptr<renderer> renderer);
  ~canvas() = default;

  canvas(const canvas&) = delete;
  canvas& operator=(const canvas&) = delete;
  canvas(canvas&&) noexcept = default;
  canvas& operator=(canvas&&) noexcept = default;

  void set_pixels(const char* pixels);

  void clear();

  void draw() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
  std::unique_ptr<uint32_t[]> _transparent;
};
}
