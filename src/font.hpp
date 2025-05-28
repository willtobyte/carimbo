#pragma once

#include "common.hpp"

#include "pixmap.hpp"
#include "point.hpp"

namespace graphics {
using glyphmap = std::map<uint8_t, geometry::rectangle>;

class fonteffect {
  public:
    enum class type: uint8_t {
      cursor,
      fadein
    };

    virtual ~fonteffect() = default;

    virtual void set(const std::string &text, geometry::point position) = 0;

    virtual void update(float_t delta) = 0;

    // virtual void reset() = 0;

    virtual float_t scale() { return 1.f; };

    virtual double_t angle() { return .0L; };

    virtual enum reflection reflection() { return reflection::none; };

    virtual uint8_t alpha() { return 255; };

    virtual std::string text() { return _text; };

    protected:
      std::string _text;
      geometry::point _position;
};

class fadeineffect : public fonteffect {
  public:
    virtual ~fadeineffect() = default;

    virtual void set(const std::string &text, geometry::point position) override;

    virtual void update(float_t delta) override;

    virtual uint8_t alpha() override;

  private:
    uint8_t _alpha;
    const float_t _fade_duration = .2f;
    float_t _fade_time = 0.0f;
    bool _animating = false;
    char _last_char = '\0';
    size_t _last_length = 0;
    size_t _draw_calls = 0;
};

class cursoreffect : public fonteffect {
  public:
    ~cursoreffect() override = default;

    void set(const std::string& text, geometry::point position) override {
      _base_text = text;
      _show_cursor = true;
      _last_toggle = SDL_GetTicks();
      _position = position;
      _text = _base_text + '?';
    }

    void update(float_t /*delta*/) override {
      const uint32_t now = SDL_GetTicks();
      if (now - _last_toggle < 500) return;
      _last_toggle = now;
      _show_cursor = !_show_cursor;
      _text = _show_cursor ? (_base_text + '?') : _base_text;
    }

  private:
    std::string _base_text;
    bool _show_cursor = true;
    uint32_t _last_toggle = 0;
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

  mutable std::string _last_text;
  mutable geometry::point _last_position;

  std::unique_ptr<fonteffect> _effect;
};
}
