#include "soundmanager.hpp"

using namespace audio;

soundmanager::soundmanager(const std::shared_ptr<audiodevice> audiodevice) noexcept
    : _audiodevice(std::move(audiodevice)) {}

std::shared_ptr<soundfx> soundmanager::get(const std::string &filename) noexcept {
  if (auto it = _pool.find(filename); it != _pool.end()) [[likely]] {
    return it->second;
  }

  fmt::print("[soundmanager] cache miss {}\n", filename);

  assert(_audiodevice);

  auto ptr = std::make_shared<soundfx>(_audiodevice, filename);
  _pool.emplace(filename, ptr);

  return ptr;
}

void soundmanager::play(const std::string &filename) noexcept {
  if (const auto &sound = get("blobs/" + filename + ".ogg"); sound) {
    sound->play();
  }
}

void soundmanager::stop(const std::string &filename) noexcept {
  if (const auto &sound = get(filename); sound) {
    sound->stop();
  }
}

void soundmanager::flush() noexcept {
  const auto count = _pool.size();
  fmt::print("[soundmanager] actual size {}\n", count);
  _pool.clear();
  fmt::print("[soundmanager] {} objects have been flushed\n", count);
}
