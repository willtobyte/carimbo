#pragma once

#include "common.hpp"

#include "geometry.hpp"

class pixmap;
class renderer;
struct glypheffect;

struct alignas(32) glyphprops final {
  float u0, v0, u1, v1;
  float scaled_w, scaled_h;
  float w;
  bool valid;
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
    const boost::unordered_flat_map<size_t, glypheffect>& effects
  ) const;

  std::string_view glyphs() const noexcept;

private:
  int16_t _spacing{0};
  int16_t _leading{0};
  float _scale{1.0f};
  float _height{0.0f};
  std::shared_ptr<pixmap> _pixmap;
  std::shared_ptr<renderer> _renderer;
  std::string _glyphs;
  std::array<glyphprops, 256> _props;
  mutable boost::container::small_vector<SDL_Vertex, 512> _vertices;
  mutable boost::container::small_vector<int32_t, 768> _indices;
};
