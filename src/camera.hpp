#pragma once

#include "common.hpp"

namespace graphics {
class camera final {
  public:
    camera() = default;
    ~camera() = default;

    void update(float delta);

    std::tuple<float, float> viewport() const noexcept;

    void set_onrequestposition(sol::protected_function fn);

  private:
    std::function<std::tuple<float, float>(float)> _on_request_position;
};
}
