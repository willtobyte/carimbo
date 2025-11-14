#include "camera.hpp"

using namespace graphics;

camera::camera() noexcept {
}

void camera::update(float delta) {
  if (const auto& fn = _on_request_calc; fn) {
    std::tie(_x, _y, _width, _height) = fn(delta);
  }
}

void camera::set_onrequestcalc(sol::protected_function fn) {
  _on_request_calc = interop::wrap_fn<std::tuple<float, float, float, float>(float)>(fn);
}

geometry::rectangle camera::viewport() const noexcept {
  return geometry::rectangle{
    _x, _y,
    _width,
    _height
  };
}

/*
local screen_width = viewport.width
local screen_height = viewport.height

local zoom = 0.5
local inv_zoom = 1 / zoom
local vw = screen_width * inv_zoom
local vh = screen_height * inv_zoom
local half_vw = vw / 2
local half_vh = vh / 2

local function calculate(delta)
  local x = player.x - half_vw
  local y = player.y - half_vh

  return x, y, vw, vh
end

camera:on_request_calc(calculate)
*/
