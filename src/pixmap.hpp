#pragma once

#include "common.hpp"
#include "rect.hpp"
#include "reflection.hpp"
#include "renderer.hpp"
#include "size.hpp"

namespace graphics {
typedef std::unique_ptr<SDL_Texture, SDL_Deleter> texture_ptr;

class pixmap {
public:
  pixmap() = default;
  pixmap(const std::shared_ptr<renderer> &renderer, const std::string &filename);
  pixmap(const std::shared_ptr<renderer> &renderer, std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> surface);
  ~pixmap() noexcept = default;

  void draw(
      const geometry::rect &source,
      const geometry::rect &destination,
      double_t angle = 0.0f,
      reflection reflection = reflection::none,
      uint8_t alpha = 255
#ifdef HITBOX
      ,
      const std::optional<geometry::rect> &outline = std::nullopt
#endif
  ) const noexcept;

  operator SDL_Texture *() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  texture_ptr _texture;
};
}
