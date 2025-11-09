#include "camera.hpp"

using namespace graphics;

void camera::update(float delta) {
  if (const auto& fn = _on_request_position; fn) {
    const auto [x, y] = fn(delta);

    std::println("camera at {} {}", x, y);
  }
}

void camera::set_onrequestposition(sol::protected_function fn) {
  _on_request_position = interop::wrap_fn<std::tuple<float, float>(float)>(std::move(fn));
}

std::tuple<float, float> camera::viewport() const {
  return std::make_tuple(1.1f, 2.2f);
}
/*

local function calculateCameraPosition(delta)
  const w, h = camera.viewport
  return 90, 10
end

camera:on_position(calculateCameraPosition)

*/
