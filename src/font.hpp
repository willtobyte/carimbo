#pragma once

#include "common.hpp"

#include "geometry.hpp"

class pixmap;

using glyphmap = std::map<char, quad>;

class font final {
public:
  font() = delete;

  explicit font(
    std::string_view glyphs,
    const glyphmap& map,
    std::shared_ptr<pixmap> pixmap,
    int16_t spacing,
    int16_t leading,
    float scale
  );

  ~font() = default;

  void draw(std::string_view text, const vec2& position, const std::weak_ptr<fonteffect>& effect) const;

  std::string_view glyphs() const;

private:
  int16_t _spacing{0};
  int16_t _leading{0};
  float _scale{1.0f};
  glyphmap _map;
  std::string _glyphs;
  std::shared_ptr<pixmap> _pixmap;
};
