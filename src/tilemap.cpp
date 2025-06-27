#include "tilemap.hpp"

using namespace framework;

tilemap::tilemap(std::shared_ptr<graphics::renderer> renderer, std::shared_ptr<resourcemanager> resourcemanager, const std::string& name) {
  UNUSED(name);

  int32_t lw, lh;
  SDL_RendererLogicalPresentation mode;
  SDL_GetRenderLogicalPresentation(*renderer, &lw, &lh, &mode);

  float_t sx, sy;
  SDL_GetRenderScale(*renderer, &sx, &sy);

  const auto width = static_cast<float_t>(lw) / sx;
  const auto height = static_cast<float_t>(lh) / sy;

  _view = { .0f, .0f, width, height };

  _pixmap = resourcemanager->pixmappool()->get(fmt::format("blobs/tilesets/{}.png", name));

  const auto& filename = fmt::format("cursors/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  _size = j["size"].template get<float_t>();
  _height = j["height"].template get<float_t>();
  _width = j["width"].template get<float_t>();

  _layers = j.at("layers")
              .get<std::vector<std::vector<uint8_t>>>();

  _labels = j["labels"].get<std::vector<std::string>>();
  /*
  {
    "size": 16,
    "height": 2,
    "width", 6,
    "layers": [
      [1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1],
      [0, 2, 0, 2, 0, 2, 0, 2, 0, 2, 2, 0],
      [2, 2, 0, 2, 2, 2, 2, 2, 2, 2, 2, 0]
    ],
  }
  */



  const auto tiles_per_row = static_cast<uint32_t>(_pixmap->width()) / static_cast<uint32_t>(_size);

  static constexpr auto max_index = std::numeric_limits<uint8_t>::max();

  _sources.resize(max_index + 1);
  _sources[0] = geometry::rectangle{{-1.f, -1.f}, {0.f, 0.f}};
  for (uint16_t i = 1; i <= max_index; ++i) {
    const auto zbi = static_cast<uint32_t>(i - 1);
    const auto src_x = static_cast<float_t>(zbi % tiles_per_row) * _size;
    const auto src_y = std::floor(static_cast<float_t>(zbi) / static_cast<float_t>(tiles_per_row)) * _size;

    _sources[i] = geometry::rectangle{{src_x, src_y}, {_size, _size}};
  }
}

void tilemap::update(float_t delta) noexcept {
  UNUSED(delta);

  if (!_target) [[unlikely]] {
    return;
  }

  const auto position = _target->position();
  const auto vw = _view.width();
  const auto vh = _view.height();

  _view.set_position(position.x() - vw * 0.5f, position.y() - vh * 0.5f);
}

void tilemap::draw() const noexcept {
  if (!_pixmap) [[unlikely]] {
    return;
  }

  const auto view_x0 = _view.x();
  const auto view_y0 = _view.y();
  const auto view_x1 = view_x0 + _view.width();
  const auto view_y1 = view_y0 + _view.height();

  for (const auto& layer : _layers) {
    const auto layer_size = layer.size();
    const auto tiles_per_row = static_cast<size_t>(_width);

    for (size_t i = 0; i < layer_size; ++i) {
      const auto index = layer[i];
      if (!index || index >= _sources.size()) [[unlikely]] {
        continue;
      }

      const auto column = i % tiles_per_row;
      const auto row = i / tiles_per_row;

      const auto tile_x = static_cast<float_t>(column) * _size;
      const auto tile_y = static_cast<float_t>(row) * _size;

      if (tile_x + _size < view_x0) [[likely]] continue;
      if (tile_x > view_x1) [[likely]] continue;
      if (tile_y + _size < view_y0) [[likely]] continue;
      if (tile_y > view_y1) [[likely]] continue;

      const auto& source = _sources[index];

      const geometry::rectangle destination{{ tile_x - view_x0, tile_y - view_y0 }, { _size, _size }};

      _pixmap->draw(source, destination);
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  if (!object) [[unlikely]] {
    return;
  }

  _target = std::move(object);
}
