#pragma once

#include "common.hpp"

#include "parallax.hpp"
#include "scene.hpp"
#include "tilemap.hpp"

class tilemapscene final : public scene {
public:
  tilemapscene(
      std::string_view name,
      unmarshal::document& document,
      std::weak_ptr<::scenemanager> scenemanager
  );

  ~tilemapscene() noexcept override = default;

  void update(float delta) noexcept override;

  void draw() const noexcept override;


  void set_oncamera(sol::protected_function&& fn) override;

  // void set_camera(vec2 position) noexcept;

  // [[nodiscard]] vec2 camera() const noexcept;

private:
  tilemap _tilemap;
  parallax _parallax;
  vec3 _camera{0.f, 0.f, 1.f};
  int _width{};
  int _height{};
  functor _oncamera;
};
