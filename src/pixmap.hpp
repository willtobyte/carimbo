#pragma once

#include "common.hpp"
#include "rectangle.hpp"
#include "reflection.hpp"
#include "renderer.hpp"
#include "size.hpp"

namespace graphics {
class pixmap {
public:
  pixmap() = delete;
  pixmap(std::shared_ptr<renderer> renderer, const std::string &filename);
  pixmap(std::shared_ptr<renderer> renderer, std::unique_ptr<SDL_Surface, SDL_Deleter> surface);
  ~pixmap() noexcept = default;

  void draw(
      const geometry::rectangle &source,
      const geometry::rectangle &destination,
      double_t angle = 0.0f,
      reflection reflection = reflection::none,
      uint8_t alpha = 255
#ifdef HITBOX
      ,
      const std::optional<geometry::rectangle> &outline = std::nullopt
#endif
  ) const noexcept;

  operator SDL_Texture *() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _texture;
};
}
