#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(std::shared_ptr<audiodevice> audiodevice) noexcept
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string& filename) noexcept {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  std::println("[soundmanager] cache miss {}", filename);

  assert(_audiodevice);

  auto ptr = std::make_shared<soundfx>(filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void soundmanager::play(const std::string& filename, bool loop) noexcept {
  if (const auto& sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->play(loop);
  }
}

void soundmanager::stop(const std::string& filename) noexcept {
  if (const auto& sound = get(std::format("blobs/{}.ogg", filename)); sound) {
    sound->stop();
  }
}

void soundmanager::flush() noexcept {
  std::println("[soundmanager] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto& pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  std::println("[soundmanager] {} objects have been flushed", count);
}
