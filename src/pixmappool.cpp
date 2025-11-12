#include "pixmappool.hpp"

#include "io.hpp"
#include "pixmap.hpp"
#include "renderer.hpp"

using namespace graphics;

pixmappool::pixmappool(std::shared_ptr<renderer> renderer)
    : _renderer(std::move(renderer)) {}

std::shared_ptr<pixmap> pixmappool::get(std::string_view filename) {
  const auto [it, inserted] = _pool.try_emplace(std::string(filename));
  if (!inserted) [[unlikely]] {
    return it->second;
  }

  std::println("[pixmappool] cache miss {}", filename);

  return it->second = std::make_shared<pixmap>(_renderer, filename);
}

void pixmappool::flush() {
  std::println("[pixmappool] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[pixmappool] {} objects have been flushed", count);
}

#ifndef NDEBUG
void pixmappool::debug() const {
  std::println("[pixmappool.debug] total objects: {}", _pool.size());

  for (const auto& [key, ptr] : _pool) {
    std::println("  {} use_count={}", key, ptr.use_count());
  }
}
#endif
