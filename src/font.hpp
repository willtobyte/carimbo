#pragma once

#include "common.hpp"

#include "geometry.hpp"

class pixmap;
class renderer;
struct glyphprops;

struct alignas(16) glyph_uv final {
  float u0, v0, u1, v1;
};

using glyphmap = std::array<std::optional<quad>, 256>;

class font final {
public:
  font() = delete;

  explicit font(
    std::string_view glyphs,
    const glyphmap& map,
    std::shared_ptr<pixmap> pixmap,
    std::shared_ptr<renderer> renderer,
    int16_t spacing,
    int16_t leading,
    float scale
  );

  ~font() = default;

  void draw(
    std::string_view text,
    const vec2& position,
    const boost::unordered_flat_map<size_t, glyphprops>& effects
  ) const;

  std::string_view glyphs() const noexcept;

private:
  int16_t _spacing{0};
  int16_t _leading{0};
  float _scale{1.0f};
  glyphmap _map;
  std::array<glyph_uv, 256> _uv_table{};
  std::string _glyphs;
  std::shared_ptr<pixmap> _pixmap;
  std::shared_ptr<renderer> _renderer;
  mutable std::vector<SDL_Vertex> _vertices;
  mutable std::vector<int32_t> _indices;
};