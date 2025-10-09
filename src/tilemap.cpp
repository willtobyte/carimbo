#include "tilemap.hpp"

using namespace framework;

tilemap::tilemap(
    geometry::size size,
    std::shared_ptr<resourcemanager> resourcemanager,
    const std::string& name) {
  _view = { 0.f, 0.f, size.width(), size.height() };

  _pixmap = resourcemanager->pixmappool()->get(std::format("blobs/tilemaps/{}.png", name));

  const auto& filename = std::format("tilemaps/{}.json", name);
  const auto& buffer = storage::io::read(filename);
  const auto& j = nlohmann::json::parse(buffer);

  _size = j["size"].get<float>();
  _height = j["height"].get<float>();
  _width = j["width"].get<float>();
  _layers = j["layers"].get<std::vector<std::vector<uint8_t>>>();
  _visibles = j.value("visibles", std::vector<bool>{});
  _labels = j.value("labels", std::vector<std::string>{});
  _transactions = j.value("transactions", std::vector<transaction>{});

  const auto tpr = static_cast<uint32_t>(_pixmap->width()) / static_cast<uint32_t>(_size);
  static constexpr auto max_index = std::numeric_limits<uint8_t>::max();

  _sources.resize(max_index + 1);
  _sources[0] = {{-1.f, -1.f}, {0.f, 0.f}};
  for (uint16_t i = 1; i <= max_index; ++i) {
    const auto idx = static_cast<uint32_t>(i - 1);
    const auto src_x = static_cast<float>(idx % tpr) * _size;
    const auto src_y = std::floor(static_cast<float>(idx) / static_cast<float>(tpr)) * _size;
    _sources[i] = {{src_x, src_y}, {_size, _size}};
  }
}

void tilemap::update(float delta) noexcept {
  UNUSED(delta);

  const auto now = SDL_GetTicks();

  if (_last_tick == 0) {
    _last_tick = now;
  }

  if (!_transactions.empty()) {
    const auto& transaction = _transactions[0];
    if (!transaction.path.empty() && (now - _last_tick >= transaction.delay)) {
      const auto count = transaction.path.size();
      const auto prev = _current_transaction % count;
      const auto next = (_current_transaction + 1) % count;

      const auto disable_it = std::find(_labels.begin(), _labels.end(), transaction.path[prev]);
      if (disable_it != _labels.end()) [[likely]] {
        _visibles[static_cast<size_t>(std::distance(_labels.begin(), disable_it))] = false;
      }

      const auto enable_it = std::find(_labels.begin(), _labels.end(), transaction.path[next]);
      if (enable_it != _labels.end()) [[likely]] {
        _visibles[static_cast<size_t>(std::distance(_labels.begin(), enable_it))] = true;
      }

      _current_transaction = next;
      _last_tick = now;
    }
  }

  if (_target) [[likely]] {
    const auto position = _target->position();

    _view.set_position(
      position.x() - _view.width() * 0.5f,
      position.y() - _view.height() * 0.5f
    );
  }
}

void tilemap::draw() const noexcept {
  if (!_pixmap) [[unlikely]] {
    return;
  }

  const auto view_x0 = _view.x();
  const auto view_y0 = _view.y();
  const auto view_x1 = view_x0 + _view.width();
  const auto view_y1 = view_y0 + _view.height();

  const auto minc = static_cast<size_t>(std::max(0.f, std::floor(view_x0 / _size)));
  const auto maxc = std::min(static_cast<size_t>(_width), static_cast<size_t>(std::ceil(view_x1 / _size)));

  const auto minr = static_cast<size_t>(std::max(0.f, std::floor(view_y0 / _size)));
  const auto maxr = std::min(static_cast<size_t>(_height), static_cast<size_t>(std::ceil(view_y1 / _size)));

  const auto tpr = static_cast<size_t>(_width);

  for (size_t l = 0; l < _layers.size(); ++l) {
    if (l < _visibles.size() && !_visibles[l]) continue;

    const auto& layer = _layers[l];
    const auto layer_size = layer.size();

    for (size_t row = minr; row < maxr; ++row) {
      for (size_t column = minc; column < maxc; ++column) {
        const auto i = row * tpr + column;
        if (i >= layer_size) continue;

        const auto index = layer[i];
        if (!index || index >= _sources.size()) [[unlikely]] continue;

        const auto& source = _sources[index];

        const geometry::rectangle destination{
          {static_cast<float>(column) * _size - view_x0, static_cast<float>(row) * _size - view_y0},
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

std::vector<std::string> tilemap::under() const noexcept {
  if (!_target || _layers.empty()) [[unlikely]] {
    return {};
  }

  const auto& action = _target->action();
  if (action.empty()) [[unlikely]] {
    return {};
  }

  const auto& animations = _target->_animations;
  const auto ait = animations.find(action);
  if (ait == animations.end()) [[unlikely]] {
    return {};
  }

  const auto& animation = ait->second;
  if (!animation.hitbox) [[unlikely]] {
    return {};
  }

  const auto hitbox =
    geometry::rectangle{
      _target->position() + animation.hitbox->rectangle.position() * _target->scale(),
      animation.hitbox->rectangle.size() * _target->scale()
    };

  const auto x0 = hitbox.x();
  const auto y0 = hitbox.y();
  const auto x1 = x0 + hitbox.width();
  const auto y1 = y0 + hitbox.height();

  const auto cstart = static_cast<size_t>(x0 / _size);
  const auto cend   = static_cast<size_t>(std::min(std::ceil(x1 / _size), _width));
  const auto rstart = static_cast<size_t>(y0 / _size);
  const auto rend   = static_cast<size_t>(std::min(std::ceil(y1 / _size), _height));
  const auto tpr = static_cast<size_t>(_width);

  std::vector<std::string> result;
  result.reserve(_labels.size());

  const auto vc = std::min(_layers.size(), _visibles.size());

  const auto lc = std::min(_layers.size(), _labels.size());

  for (size_t l = 0; l < vc; ++l) {
    if (!_visibles[l]) [[unlikely]] continue;

    const auto& layer = _layers[l];
    const auto ls = layer.size();

    for (auto row = rstart; row < rend; ++row) {
      const auto base = row * tpr;
      if (base >= ls) [[unlikely]] break;

      for (auto col = cstart; col < cend; ++col) {
        const auto index = base + col;
        if (index >= ls) [[unlikely]] break;

        if (const auto tile = layer[index]; tile) [[likely]] {
          if (l < lc) result.emplace_back(_labels[l]);
          goto next;
        }
      }
    }

  next:
    continue;
  }

  return result;
}
