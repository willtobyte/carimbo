#include "parallax.hpp"

#include "geometry.hpp"
#include "pixmap.hpp"
#include "pixmappool.hpp"
#include "renderer.hpp"
#include "resourcemanager.hpp"

void from_json(unmarshal::value json, repeat& out) {
  out.x = unmarshal::value_or(json, "x", true);
  out.y = unmarshal::value_or(json, "y", false);
}

void from_json(unmarshal::value json, parallax_layer& out) {
  if (unmarshal::contains(json, "speed")) {
    out.speed = unmarshal::make<vec2>(json["speed"]);
  }
  out.z_order = unmarshal::value_or(json, "z", static_cast<int8_t>(0));
  if (unmarshal::contains(json, "repeat")) {
    out.repeat = unmarshal::make<struct repeat>(json["repeat"]);
  }
}

parallax::parallax(
    std::string_view scene,
    unmarshal::document& document,
    std::shared_ptr<resourcemanager> resourcemanager)
    : _renderer(resourcemanager->renderer()) {
  SDL_GetRenderOutputSize(*_renderer, &_width, &_height);

  if (auto layers = unmarshal::find_array(document, "parallax")) {
    const auto pixmappool = resourcemanager->pixmappool();

    for (auto element : *layers) {
      parallax_layer layer = unmarshal::make<parallax_layer>(element);
      const auto texture_name = unmarshal::get<std::string_view>(element, "texture");
      layer.texture = pixmappool->get(
          std::format("blobs/{}/parallax/{}.png", scene, texture_name));

      if (layer.z_order < 0) {
        _back_layers.push_back(std::move(layer));
      } else {
        _front_layers.push_back(std::move(layer));
      }
    }

    std::ranges::sort(_back_layers, {}, &parallax_layer::z_order);
    std::ranges::sort(_front_layers, {}, &parallax_layer::z_order);
  }
}

void parallax::set_camera(vec2 position) noexcept {
  _camera = position;

  for (auto& layer : _back_layers) {
    layer.offset = _camera * layer.speed;
  }

  for (auto& layer : _front_layers) {
    layer.offset = _camera * layer.speed;
  }
}

void parallax::update([[maybe_unused]] float delta) noexcept {
}

void parallax::draw_back() const noexcept {
  for (const auto& layer : _back_layers) {
    draw_layer(layer);
  }
}

void parallax::draw_front() const noexcept {
  for (const auto& layer : _front_layers) {
    draw_layer(layer);
  }
}

void parallax::draw_layer(const parallax_layer& layer) const noexcept {
  const auto tex_w = static_cast<float>(layer.texture->width());
  const auto tex_h = static_cast<float>(layer.texture->height());

  const auto wrap_offset_x = layer.repeat.x
      ? std::fmod(layer.offset.x, tex_w)
      : layer.offset.x;
  const auto wrap_offset_y = layer.repeat.y
      ? std::fmod(layer.offset.y, tex_h)
      : layer.offset.y;

  if (layer.repeat.x || layer.repeat.y) {
    const int tiles_x = layer.repeat.x
        ? static_cast<int>(static_cast<float>(_width) / tex_w) + 2
        : 1;
    const int tiles_y = layer.repeat.y
        ? static_cast<int>(static_cast<float>(_height) / tex_h) + 2
        : 1;

    for (int ty = 0; ty < tiles_y; ++ty) {
      for (int tx = 0; tx < tiles_x; ++tx) {
        const auto dx = static_cast<float>(tx) * tex_w - wrap_offset_x;
        const auto dy = static_cast<float>(ty) * tex_h - wrap_offset_y;

        layer.texture->draw(
            0.f, 0.f, tex_w, tex_h,
            dx, dy, tex_w, tex_h
        );
      }
    }
  } else {
    layer.texture->draw(
        0.f, 0.f, tex_w, tex_h,
        -wrap_offset_x, -wrap_offset_y, tex_w, tex_h
    );
  }
}
