#pragma once

#include "common.hpp"

class canvas final {
public:
  canvas();
  ~canvas() noexcept = default;

  void set_pixels(std::string_view pixels) noexcept;

  void clear() noexcept;

  void draw() const noexcept;

private:
  std::unique_ptr<SDL_Texture, SDL_Deleter> _framebuffer;
  std::unique_ptr<uint32_t[]> _transparent;
};
