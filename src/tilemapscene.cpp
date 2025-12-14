#include "tilemapscene.hpp"

#include "geometry.hpp"
#include "scenemanager.hpp"
#include <SDL3/SDL_render.h>

tilemapscene::tilemapscene(
  std::string_view name,
  unmarshal::document& document,
  std::weak_ptr<::scenemanager> scenemanager
)
    : scene(name, document, scenemanager),
      _tilemap(unmarshal::get<std::string_view>(document, "tilemap"), scenemanager.lock()->resourcemanager()),
      _parallax(name, document, scenemanager.lock()->resourcemanager()) {
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

