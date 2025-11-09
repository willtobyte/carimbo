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
-- camera.lua

local w, h = camera.viewport

local function calculate(delta)
  return player.x + 90 + w, player.y + 10 + h
end

camera:on_request_position(calculate)

*/
