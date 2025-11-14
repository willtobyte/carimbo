#pragma once

#include "common.hpp"

namespace graphics {
class camera final {
  public:
    camera() noexcept = default;
    ~camera() noexcept = default;

    void update(float delta);

    geometry::rectangle viewport() const noexcept;

    void set_onrequestcalc(sol::protected_function fn);

  private:
    float _x{.0f};
    float _y{.0f};
    float _width{.0f};
    float _height{.0f};

    std::function<std::tuple<float, float, float, float>(float)> _on_request_calc;
};
}
