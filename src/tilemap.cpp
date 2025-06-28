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
  _view = { 0.f, 0.f, width, height };

  _pixmap = resourcemanager->pixmappool()->get(fmt::format("blobs/tilemaps/{}.png", name));

  const auto& j = nlohmann::json::parse(storage::io::read(fmt::format("tilemaps/{}.json", name)));

  _size = j["size"].get<float_t>();
  _height = j["height"].get<float_t>();
  _width = j["width"].get<float_t>();
  _layers = j["layers"].get<std::vector<std::vector<uint8_t>>>();
  _visibles = j.value("visibles", std::vector<bool>{});
  _labels = j.value("labels", std::vector<std::string>{});
  _transactions = j.value("transactions", std::vector<transaction>{});

  const auto tiles_per_row = static_cast<uint32_t>(_pixmap->width()) / static_cast<uint32_t>(_size);
  static constexpr auto max_index = std::numeric_limits<uint8_t>::max();

  _sources.resize(max_index + 1);
  _sources[0] = {{-1.f, -1.f}, {0.f, 0.f}};
  for (uint16_t i = 1; i <= max_index; ++i) {
    const auto idx = static_cast<uint32_t>(i - 1);
    const auto src_x = static_cast<float_t>(idx % tiles_per_row) * _size;
    const auto src_y = std::floor(static_cast<float_t>(idx) / static_cast<float_t>(tiles_per_row)) * _size;
    _sources[i] = {{src_x, src_y}, {_size, _size}};
  }
}

void tilemap::update(float_t delta) noexcept {
  UNUSED(delta);
  if (!_target) [[unlikely]] return;

  const auto pos = _target->position();
  _view.set_position(pos.x() - _view.width() * 0.5f, pos.y() - _view.height() * 0.5f);

  if (_transactions.empty()) return;
  const auto& transaction = _transactions[0];
  if (transaction.path.empty()) return;

  const auto now = SDL_GetTicks();
  if (_last_tick == 0 || now - _last_tick < transaction.delay) {
    if (_last_tick == 0) _last_tick = now;
    return;
  }

  if (_visibles.size() < _labels.size()) _visibles.resize(_labels.size(), false);

  const auto count = transaction.path.size();
  const auto prev = _current_transaction % count;
  const auto next = (_current_transaction + 1) % count;

  const auto disable_it = std::find(_labels.begin(), _labels.end(), transaction.path[prev]);
  if (disable_it != _labels.end())
    _visibles[static_cast<size_t>(std::distance(_labels.begin(), disable_it))] = false;

  const auto enable_it = std::find(_labels.begin(), _labels.end(), transaction.path[next]);
  if (enable_it != _labels.end())
    _visibles[static_cast<size_t>(std::distance(_labels.begin(), enable_it))] = true;

  _current_transaction = next;
  _last_tick = now;
}

void tilemap::draw() const noexcept {
  if (!_pixmap) [[unlikely]] {
    return;
  }

  const auto view_x0 = _view.x();
  const auto view_y0 = _view.y();
  const auto view_x1 = view_x0 + _view.width();
  const auto view_y1 = view_y0 + _view.height();

  const auto min_column = static_cast<size_t>(std::max(0.f, std::floor(view_x0 / _size)));
  const auto max_column = std::min(static_cast<size_t>(_width), static_cast<size_t>(std::ceil(view_x1 / _size)));

  const auto min_row = static_cast<size_t>(std::max(0.f, std::floor(view_y0 / _size)));
  const auto max_row = std::min(static_cast<size_t>(_height), static_cast<size_t>(std::ceil(view_y1 / _size)));

  const auto tiles_per_row = static_cast<size_t>(_width);

  for (size_t l = 0; l < _layers.size(); ++l) {
    if (l < _visibles.size() && !_visibles[l]) continue;

    const auto& layer = _layers[l];
    const auto layer_size = layer.size();

    for (size_t row = min_row; row < max_row; ++row) {
      for (size_t column = min_column; column < max_column; ++column) {
        const auto i = row * tiles_per_row + column;
        if (i >= layer_size) continue;

        const auto index = layer[i];
        if (!index || index >= _sources.size()) [[unlikely]] continue;

        const auto& source = _sources[index];

        const geometry::rectangle destination{
          {static_cast<float_t>(column) * _size - view_x0, static_cast<float_t>(row) * _size - view_y0},
          {_size, _size}
        };

        _pixmap->draw(source, destination);
      }
    }
  }
}

void tilemap::set_target(std::shared_ptr<object> object) {
  if (!object) [[unlikely]] return;
  _target = std::move(object);
}
