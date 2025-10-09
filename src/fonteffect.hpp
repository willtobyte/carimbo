#pragma once

#include "common.hpp"

namespace graphics {
class fonteffect {
  public:
    enum class type: uint8_t {
      fadein
    };

    virtual ~fonteffect() = default;

    virtual void set(const std::string& text, geometry::point position) = 0;

    virtual void update(float delta) noexcept = 0;

    virtual float scale() noexcept { return 1.f; };

    virtual double angle() noexcept { return .0L; };

    virtual reflection reflection() noexcept { return reflection::none; };

    virtual uint8_t alpha() noexcept { return 255; };
};

class fadeineffect final : public fonteffect {
  public:
    virtual ~fadeineffect() = default;

    virtual void set(const std::string& text, geometry::point position) override;

    virtual void update(float delta) noexcept override;

    virtual uint8_t alpha() noexcept override;

  private:
    std::string _text;
    geometry::point _position;
    uint8_t _alpha;
    float _fade_time{.0f};
    bool _animating{false};
    size_t _last_length{0};
    size_t _draw_calls{0};

    const float _fade_duration{.3f};
};
}
