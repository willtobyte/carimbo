#include "camera.hpp"

using namespace graphics;

void camera::update(float delta) {
  if (const auto& fn = _request_position; fn) {
    const auto [x, y] = fn(delta);

    std::println("camera at {} {}", x, y);
  }
}

void camera::set_onrequestposition(sol::protected_function fn) {
  _request_position = interop::wrap_fn<std::tuple<float, float>(float)>(std::move(fn));
}

/*

local function calculateCameraPosition(delta)
  return 90, 10
end

camera:on_request_position(calculateCameraPosition)

*/
