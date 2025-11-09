#include "camera.hpp"

using namespace graphics;

void camera::update(float delta) {
  if (const auto& fn = _on_camera; fn) {
    const auto [x, y] = fn(delta);

    std::println("camera at {} {}", x, y);
  }
}

void camera::set_oncamera(sol::protected_function fn) {
  _on_camera = interop::wrap_fn<std::tuple<float, float>(float)>(std::move(fn));
}
