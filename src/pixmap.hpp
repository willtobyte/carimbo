#pragma once

#include "common.hpp"

#include "rectangle.hpp"
#include "reflection.hpp"
#include "renderer.hpp"

namespace graphics {
enum class blendmode : SDL_BlendMode {
  None = SDL_BLENDMODE_NONE,
  Add  = SDL_BLENDMODE_ADD
};

class pixmap final {
public:
  pixmap() = delete;
  pixmap(std::shared_ptr<renderer> renderer, const std::string& filename);
  ~pixmap() noexcept = default;

  void draw(
      const geometry::rectangle& source,
      const geometry::rectangle& destination,
      double_t angle = 0.0L,
      reflection reflection = reflection::none,
      uint8_t alpha = 255
#ifdef DEBUG
      ,
      const std::optional<geometry::rectangle>& outline = std::nullopt
#endif
  ) const noexcept;

  operator SDL_Texture *() const noexcept;

  int32_t width() const noexcept;

  int32_t height() const noexcept;

private:
  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _texture;

  int32_t _width;
  int32_t _height;
};
}
