#pragma once

#include "common.hpp"

namespace graphics {
class camera final {
  public:

    void update(float delta);

    void set_oncamera(sol::protected_function fn);

  private:
    std::function<std::tuple<float, float>(float)> _on_camera;
};
}
