#pragma once

#include "common.hpp"

namespace graphics {
class camera final {
  public:
    camera() = default;
    ~camera() = default;
    camera(const camera&) = default;
    camera& operator=(const camera&) = default;
    camera(camera&&) noexcept = default;
    camera& operator=(camera&&) noexcept = default;

    void update(float delta);

    std::tuple<float, float> viewport() const noexcept;

    void set_onrequestposition(sol::protected_function fn);

  private:
    std::function<std::tuple<float, float>(float)> _on_request_position;
};
}
