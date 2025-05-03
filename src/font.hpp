#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "point.hpp"

namespace graphics {
using glyphmap = std::map<uint8_t, geometry::rectangle>;

class fonteffect {
  public:
    enum class type: uint8_t {
      fadein,
    };

    virtual ~fonteffect() = default;

    virtual void update(float_t delta) = 0;

    virtual void reset() = 0;

    virtual void alpha() = 0;

    // TODO add more font props
};

class fadeineffect : public fonteffect {
  public:
    virtual ~fadeineffect() = default;

    virtual void update(float_t delta) override;

    virtual void reset() override;

    virtual void alpha() override;
};

class font final {
public:
  font() = delete;
  explicit font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale);
  ~font() = default;

  void update(float_t delta);

  void draw(const std::string &text, const geometry::point &position) const;

private:
  glyphmap _glyphs;
  std::shared_ptr<pixmap> _pixmap;
  int16_t _spacing{0};
  int16_t _leading{0};
  float_t _scale{1.0f};

  // TODO move to effect class
  float_t _fade_duration;

  mutable float_t _fade_time = 0.0f;
  mutable bool _animating = false;
  mutable char _last_char = '\0';
};
}
