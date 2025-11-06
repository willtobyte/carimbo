#pragma once

#include "common.hpp"

#include "fonteffect.hpp"
#include "point.hpp"

namespace graphics {
class pixmap;

using glyphmap = std::map<char, geometry::rectangle>;

class font final {
public:
  font() = delete;

  explicit font(
    const std::string& glyphs,
    const glyphmap& map,
    std::shared_ptr<pixmap> pixmap,
    int16_t spacing,
    int16_t leading,
    float scale
  );

  ~font() = default;

  void draw(const std::string& text, const geometry::point& position, const std::weak_ptr<fonteffect>& effect) const;

  std::string glyphs() const;

private:
  std::string _glyphs;
  glyphmap _map;
  std::shared_ptr<pixmap> _pixmap;
  int16_t _spacing{0};
  int16_t _leading{0};
  float _scale{1.0f};
};
}
