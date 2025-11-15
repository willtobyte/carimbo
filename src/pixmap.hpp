#pragma once

#include "common.hpp"

#include "reflection.hpp"

namespace graphics {

enum class blendmode : int {
  None = SDL_BLENDMODE_NONE,
  Blend = SDL_BLENDMODE_BLEND,
  Add  = SDL_BLENDMODE_ADD
};

class pixmap final {
public:
  pixmap() = delete;
  pixmap(std::shared_ptr<renderer> renderer, std::string_view filename);
  ~pixmap() = default;

  void draw(
      const geometry::point& source,
      const geometry::rectangle& destination,
      double angle = 0.0L,
      uint8_t alpha = 255,
      reflection reflection = reflection::none
  ) const;

  void draw(
      const geometry::rectangle& source,
      const geometry::rectangle& destination,
      double angle = 0.0L,
      uint8_t alpha = 255,
      reflection reflection = reflection::none
  ) const;

  operator SDL_Texture* () const;

  int width() const;

  int height() const;

  void set_blendmode(blendmode mode);

private:
  int _width;
  int _height;

  std::shared_ptr<renderer> _renderer;
  std::unique_ptr<SDL_Texture, SDL_Deleter> _texture;
};
}
