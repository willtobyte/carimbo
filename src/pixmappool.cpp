#include "pixmappool.hpp"

using namespace graphics;

pixmappool::pixmappool(std::shared_ptr<renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {}

std::shared_ptr<pixmap> pixmappool::get(const std::string &filename) {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  fmt::println("[pixmappool] cache miss {}", filename);

  assert(_renderer);

  auto ptr = std::make_shared<pixmap>(_renderer, filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void pixmappool::flush() noexcept {
  fmt::println("[pixmappool] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  fmt::println("[pixmappool] {} objects have been flushed", count);
}
