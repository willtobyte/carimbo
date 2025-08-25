#include "pixmappool.hpp"

using namespace graphics;

pixmappool::pixmappool(std::shared_ptr<renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {}

std::shared_ptr<pixmap> pixmappool::get(const std::string& name) {
  if (auto it = _pool.find(name); it != _pool.end()) [[unlikely]] {
    return it->second;
  }

  std::println("[pixmappool] cache miss {}", name);

  assert(_renderer);

  auto ptr = std::make_shared<pixmap>(_renderer, name);
  _pool.emplace(name, ptr);

  return ptr;
}

void pixmappool::flush() noexcept {
  std::println("[pixmappool] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[pixmappool] {} objects have been flushed", count);
}

#ifdef DEBUG
void pixmappool::debug() const noexcept {
  std::println("pixmappool::debug() total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("[{}] use_count={}", key, ptr.use_count());
  }
}
#endif
