#include "tilemapscene.hpp"

#include "geometry.hpp"

#include "resourcemanager.hpp"

tilemapscene::tilemapscene(
  std::string_view name,
  unmarshal::document& document,
  std::weak_ptr<::resourcemanager> resourcemanager
)
    : _tilemap(unmarshal::get<std::string_view>(document, "tilemap"), resourcemanager)
      _parallax(name, document, resourcemanager){
  SDL_GetRenderOutputSize(*_renderer, &_width, &_height);
}

void tilemapscene::update(float delta) noexcept {
  if (_oncamera) {
    _camera = _oncamera.call<vec3>(delta);
  }

  _tilemap.set_viewport({_camera.x, _camera.y, static_cast<float>(_width), static_cast<float>(_height)});
  _tilemap.update(delta);

  scene::update(delta);

  _parallax.set_camera({_camera.x, _camera.y});
  _parallax.update(delta);
}

void tilemapscene::draw() const noexcept {
  _parallax.draw_back();

  _tilemap.draw();

  scene::draw();

  _parallax.draw_front();
}

void tilemapscene::set_oncamera(sol::protected_function&& fn) {
  _oncamera = std::move(fn);
}

