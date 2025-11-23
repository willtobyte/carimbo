#pragma once

#include "common.hpp"

#include "reflection.hpp"

namespace graphics {
class fonteffect {
  public:
    enum type: uint8_t {
      fadein
    };

    virtual ~fonteffect() = default;

    virtual void set(std::string_view text, math::vec2 position) = 0;

    virtual void update(float delta) = 0;

    virtual float scale() { return 1.f; };

    virtual double angle() { return .0L; };

    virtual reflection reflection() { return reflection::none; };

    virtual uint8_t alpha() { return 255; };
};

class fadeineffect final : public fonteffect {
  public:
    virtual ~fadeineffect() = default;

    virtual void set(std::string_view text, math::vec2 position) override;

    virtual void update(float delta) override;

    virtual uint8_t alpha() override;

  private:
    uint8_t _alpha;
    float _fade_time{.0f};
    bool _animating{false};
    size_t _last_length{0};
    size_t _draw_calls{0};
    math::vec2 _position;
    std::string _text;

    const float _fade_duration{.3f};
};
}
