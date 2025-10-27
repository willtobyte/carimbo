#include "pixmappool.hpp"
#include "pixmap.hpp"

using namespace graphics;

pixmappool::pixmappool(std::shared_ptr<renderer> renderer) noexcept
    : _renderer(std::move(renderer)) {}

std::shared_ptr<pixmap> pixmappool::get(const std::string& filename) {
  const auto [it, inserted] = _pool.try_emplace(filename);
  if (!inserted) [[unlikely]] {
    return it->second;
  }

  std::println("[pixmappool] cache miss {}", filename);
  assert(_renderer);

  it->second = std::make_shared<pixmap>(_renderer, filename);
  return it->second;
}

void pixmappool::flush() noexcept {
  std::println("[pixmappool] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[pixmappool] {} objects have been flushed", count);
}

#ifndef NDEBUG
void pixmappool::debug() const noexcept {
  std::println("[pixmappool.debug] total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  {} use_count={}", key, ptr.use_count());
  }
}
#endif
