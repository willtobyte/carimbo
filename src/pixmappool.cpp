#include "pixmappool.hpp"

using namespace graphics;

pixmappool::pixmappool(const std::shared_ptr<renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {}

const std::shared_ptr<pixmap> pixmappool::get(const std::string &filename) {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  fmt::print("[pixmappool] cache miss {}\n", filename);

  assert(_renderer);

  auto ptr = std::make_shared<pixmap>(_renderer, filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void pixmappool::flush() noexcept {
  fmt::print("[pixmappool] actual size {}\n", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  fmt::print("[pixmappool] {} objects have been flushed\n", count);
}
