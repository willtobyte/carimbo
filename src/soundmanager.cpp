#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(std::shared_ptr<audiodevice> audiodevice) noexcept
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string &filename) noexcept {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  fmt::println("[soundmanager] cache miss {}", filename);

  assert(_audiodevice);

  auto ptr = std::make_shared<soundfx>(filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void soundmanager::play(const std::string &filename, bool loop) noexcept {
  if (const auto &sound = get("blobs/" + filename + ".ogg"); sound) {
    sound->play(loop);
  }
}

void soundmanager::stop(const std::string &filename) noexcept {
  if (const auto &sound = get(filename); sound) {
    sound->stop();
  }
}

void soundmanager::flush() noexcept {
  const auto count = _pool.size();
  fmt::println("[soundmanager] actual size {}", count);
  _pool.clear();
  fmt::println("[soundmanager] {} objects have been flushed", count);
}
