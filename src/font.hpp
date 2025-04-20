#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "point.hpp"

namespace graphics {
using glyphmap = std::map<uint8_t, geometry::rectangle>;

class font final {
public:
  font() = delete;
  explicit font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale);
  ~font() = default;

  void draw(const std::string &text, const geometry::point &position) const noexcept;

private:
  glyphmap _glyphs;
  std::shared_ptr<pixmap> _pixmap;
  int16_t _spacing{0};
  int16_t _leading{0};
  float_t _scale{1.0f};
};
}
