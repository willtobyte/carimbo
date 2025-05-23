#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(std::shared_ptr<audiodevice> audiodevice)
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string &filename) {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  fmt::println("[soundmanager] cache miss {}", filename);

  assert(_audiodevice);

  auto ptr = std::make_shared<soundfx>(filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void soundmanager::play(const std::string &filename, bool loop) {
  if (const auto &sound = get("blobs/" + filename + ".ogg"); sound) {
    sound->play(loop);
  }
}

void soundmanager::stop(const std::string &filename) {
  if (const auto &sound = get(filename); sound) {
    sound->stop();
  }
}

void soundmanager::flush() {
  fmt::println("[soundmanager] actual size {}", _pool.size());

  const auto count = std::erase_if(_pool, [](const auto &pair) { return pair.second.use_count() == MINIMAL_USE_COUNT; });
  fmt::println("[soundmanager] {} objects have been flushed", count);
}
