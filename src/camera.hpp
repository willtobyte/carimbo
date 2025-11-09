#pragma once

#include "common.hpp"

namespace graphics {
class camera final {
  public:

    void update(float delta);

    void set_onrequestposition(sol::protected_function fn);

  private:
    std::function<std::tuple<float, float>(float)> _request_position;
};
}
