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

    virtual void set(const std::string &text, geometry::point position) = 0;

    virtual void update(float_t delta) = 0;

    // virtual void reset() = 0;

    virtual float_t scale() { return 1.f; };

    virtual enum reflection reflection() { return reflection::none; };

    virtual uint8_t alpha() { return 255; };
};

class fadeineffect : public fonteffect {
  public:
    virtual ~fadeineffect() = default;

    virtual void set(const std::string &text, geometry::point position) override;

    virtual void update(float_t delta) override;

    virtual float_t scale() override;

    virtual enum reflection reflection() override;

    virtual uint8_t alpha() override;
  private:
    std::string _text;
    geometry::point _position;
    uint8_t _alpha;
    const float_t _fade_duration = .2f;
    float_t _fade_time = 0.0f;
    bool _animating = false;
    char _last_char = '\0';
};

class font final {
public:
  font() = delete;
  explicit font(const glyphmap &glyphs, std::shared_ptr<pixmap> pixmap, int16_t spacing, int16_t leading, float_t scale);
  ~font() = default;

  void set_effect(fonteffect::type type);

  void update(float_t delta);

  void draw(const std::string &text, const geometry::point &position) const;

private:
  glyphmap _glyphs;
  std::shared_ptr<pixmap> _pixmap;
  int16_t _spacing{0};
  int16_t _leading{0};
  float_t _scale{1.0f};

  std::unique_ptr<fonteffect> _effect;
};
}
