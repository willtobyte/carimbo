#include "pixmappool.hpp"

using namespace graphics;

pixmappool::pixmappool(const std::shared_ptr<renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {}

const std::shared_ptr<pixmap> pixmappool::get(const std::string &filename) {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  std::cout << "[pixmappool] cache miss " << filename << std::endl;

  assert(_renderer);

  auto ptr = std::make_shared<pixmap>(_renderer, filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void pixmappool::flush() noexcept {
  std::cout << "[pixmappool] actual size " << _pool.size() << std::endl;

  const auto count = std::erase_if(_pool, [](const auto &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::cout << "[pixmappool] " << count << " objects have been flushed" << std::endl;
}
